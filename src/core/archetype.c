#include "archetype.h"

archetype empty_archetype = {0};

feach(push_to_set, uint64_t, hash, {
  type_set *types = (type_set *)args;
  type_set_put(types, &hash);
});
feach(add_new_offset, kvpair, type, {
  void     **list = (void **)args;
  archetype *a = list[0];
  g_core    *w = list[1];
  gsize     *component_pos = list[2];

  gid   *type_name = type.value;
  gsize *type_size = id_to_size_get(&w->component_registry, type_name).value;

  /* Add to offset map and increment position */
  id_to_size_put(&a->offsets, type_name, type_size);
  *component_pos += *type_size;
});
void init_archetype(g_core *w, archetype *a, hash_vec *key) {

  /* We only store the actual id value because the first bit is
     meaningless. */
  a->archetype_id = SELECT_ID(gid_atomic_incr(&w->id_gen));
  a->hash_name = hash_vector(key);

  /* Init indexers and component containers */
  hash_to_size_inita(&a->offsets, w->allocator, TO_HEAP, 1);
  id_to_int64_inita(&a->entt_positions, w->allocator, TO_HEAP, 1);

  /* Init system cache */
  cache_vec_inita(&a->contenders, w->allocator, TO_HEAP, 1);

  /* Init buffers */
  id_vec_inita(&a->entt_creation_buffer, w->allocator, TO_HEAP, 1);
  id_vec_inita(&a->entt_creation_buffer, w->allocator, TO_HEAP, 1);
  int64_vec_inita(&a->dead_fragment_buffer, w->allocator, TO_HEAP, 1);

  /* Apply type set */
  type_set_hinit(&a->types);
  hash_vec_foreach(key, push_to_set, &a->types);

  /* Construct the offset table. Increment and collect the sizes of each type */
  gsize component_pos = 0;
  void *args[3];
  args[0] = &a;
  args[1] = w;
  args[2] = &component_pos;

  map_foreach(&a->types.internals, add_new_offset, args);

  /* The lenghts of an element in the composite vector is equal to the final
     position of 'component_pos' */
  __vec_init(&a->components, component_pos, w->allocator, TO_HEAP, 1);

  /* Caches don't get caches! Recursion base case here */
  if (SELECT_MODE(atomic_load(&w->id_gen)) == CACHED) return;

  /* Setup the archetypes simulation for non-concurrent properties */
  a->simulation = g_create_world();
  gid_atomic_set(&a->simulation->id_gen, CACHED);

  /* Inside the simulation context, a transition can be triggered where there
     are no archetypes inside the cache beforehand. This causes the archetype
     being transitioned use GID = 0 for its ID when generating. This is a
     huge problem because 0 represents the empty archetype. To fix this, we
     simply increment before leaving the initialization step.

     Also, to maintain entity ID consistency, entities will not use this
     id_gen to create entities. This is only used for archetype indexing. */
  gid_atomic_incr(&a->simulation->id_gen);

  /* To retain the recursive properties of all functions being used, we
     copy over the component registry data to the simulated world. */
  id_to_size_free(&a->simulation->component_registry);
  id_to_size_copy(&a->simulation->component_registry, &w->component_registry);
}

void free_archetype(archetype *a) {
  assert(a);

  type_set_free(&a->types);
  vec_free(&a->components);
  hash_to_size_free(&a->offsets);
  id_to_int64_free(&a->entt_positions);

  cache_vec_free(&a->contenders);

  id_vec_free(&a->entt_creation_buffer);
  id_vec_free(&a->entt_deletion_buffer);
  int64_vec_free(&a->dead_fragment_buffer);
};

compare(sort_hashes, int64_t, a, b, { return a < b; });
void archetype_key(char *types, hash_vec *hashes) {
  assert(types);

  /* Count how many types there are to not waste stack space on reallocs */
  char   *origin = types;
  int64_t type_count = 1; /* 1 because we assert types is not zero. */
  while (*types) {
    if (*types == ',') type_count++;
    types++;
  }
  types = origin;

  int64_vec_sinit(hashes, type_count);

  /* Add each hash to the vector */
  int64_t type_str_length = 0;
  while (*types) {
    if (*types == ',') {
      assert(type_str_length > 0);
      uint64_t type_hash = hash_bytes(types - 1, type_str_length);
      hash_vec_push(hashes, &type_hash);
      type_str_length = 0;
    }
    types++;
    type_str_length++;
  }
  types = origin;
  if (type_str_length > 0) {
    uint64_t type_hash = hash_bytes(types, type_str_length);
    hash_vec_push(hashes, &type_hash);
  }
  vec_sort(hashes, sort_hashes, NULL);
}

feach(migrate_segments, kvpair, item, {
  void     **list = (void **)args;
  void      *prev_seg = list[0];
  void      *next_seg = list[1];
  g_core    *w = list[2];
  archetype *prev = list[3];
  archetype *next = list[4];

  uint64_t *type = item.value;

  gsize *component_size = hash_to_size_get(&w->component_registry, type).value;
  gsize *prev_offset = hash_to_size_get(&prev->offsets, type).value;
  gsize *next_offset = hash_to_size_get(&next->offsets, type).value;

  memmove(next_seg + *next_offset, prev_seg + *prev_offset, *component_size);
});
void delta_transition(g_core *w, gid entt, hash_vec *to_key) {
  archetype *a_next, *a_prev;
  kvpair     kv;

  /* We generate the archetype id by hashing the key. Since the vector is known
     to be ordered, the hashes will be the same.  */
  uint64_t arch_id = hash_vector(to_key);

  /* Check if there already exists an archetype with this id. If not, make it */
  if (!id_to_archetype_has(&w->archetype_registry, &arch_id)) {
    /* Make new archetype */
    archetype a = {0};
    init_archetype(w, &a, to_key);

    /* Add the world, a new archetype appearing causes the FSM process to
       retrigger. */
    id_to_archetype_put(&w->archetype_registry, &arch_id, &a);
    w->invalidate_fsm = 1;
  }

  /* Load the archetype of the current entity */
  kv = id_to_hash_get(&w->entity_registry, &entt);
  assert(kv.value && "Invalid Entity ID!");
  a_prev = hash_to_archetype_get(&w->archetype_registry, kv.value).value;

  /* Load the archetype to transition to */
  a_next = hash_to_archetype_get(&w->archetype_registry, &arch_id).value;

  /* Case 1: a_prev is the empty archetype. We don't need to do anything other
             than move the entity to a_next. No copying is necessary. */
  if (a_prev == &empty_archetype) {
    /* Add one more space for the incomming entity to this archetype. */
    int64_t pos = a_next->components.length;
    composite_resize(&a_next->components, a_next->components.length + 1);
    memset(composite_at(&a_next->components, pos), 0,
           a_next->components.__el_size);

    /* Add the position to entity map for easy id lookup */
    id_to_int64_put(&a_next->entt_positions, &entt, &pos);

    /* Update the world entity registry for global entity access */
    // TODO: make it so that put does overwrites so we dont need to delete
    id_to_hash_del(&w->entity_registry, &entt);
    id_to_hash_put(&w->entity_registry, &entt, &arch_id);

    return;
  }

  /* Case 2: a_prev is not the empty archetype. In this case, we need to
             transition component data that we care about and discard the
             rest. We do this by collecting the intersection between the
             new and old types. */

  /* Prepare to load the previous segment */
  kv = id_to_int64_get(&a_prev->entt_positions, &entt);
  assert(kv.value && "Invalid Entity ID!");
  int64_t prev_pos = *(int *)kv.value;

  /* Prepare to load the next/new segement */
  int64_t pos = a_next->components.length;
  composite_resize(&a_next->components, a_next->components.length + 1);
  memset(composite_at(&a_next->components, pos), 0,
         a_next->components.__el_size);

  /* Load prev and next segments */
  void *prev_seg = composite_at(&a_prev->components, prev_pos);
  void *next_seg = composite_at(&a_next->components, pos);

  type_set retained_types;
  type_set_intersect(&a_prev->types, &a_next->types, &retained_types);

  /* Migrate the retained types to a_next */
  // TODO: implement some type of iterator in sets because this:
  void *args[5];
  args[0] = prev_seg;
  args[1] = next_seg;
  args[2] = w;
  args[3] = a_prev;
  args[4] = a_next;
  map_foreach(&retained_types.internals, migrate_segments, args);

  /* Cleanup reminants of the entity that was transitioned. */
  id_to_int64_del(&a_prev->entt_positions, &entt);
  id_to_int64_put(&a_next->entt_positions, &entt, &pos);
  id_to_hash_del(&w->entity_registry, &entt);
  id_to_hash_put(&w->entity_registry, &entt, &arch_id);

  /* Instead of deleting the composite vector now, we delete at the end of the
     tick as a batch process */
  int64_vec_push(&a_prev->dead_fragment_buffer, &prev_pos);
}