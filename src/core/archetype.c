#include <regex.h>

#include "archetype.h"
#include "entity.h"

archetype empty_archetype = {0};

feach(add_new_offset, kvpair, type, {
  void     **list = (void **)args;
  archetype *a = list[0];
  g_core    *w = list[1];
  gsize     *component_pos = list[2];
  gid       *type_name = type.key;
  gsize *type_size = id_to_size_get(&w->component_registry, type_name).value;
  hash_to_size_put(&a->offsets, type_name, component_pos);
  *component_pos += *type_size;
});
void init_archetype(g_core *w, archetype *a, hash_vec *key) {
  log_enter;
  /* We only store the actual id value because the first bit is
     meaningless. */
  a->archetype_id = SELECT_ID(gid_atomic_incr(&w->id_gen));
  a->hash_name = hash_vector(key);
  a->allocator = stalloc_create(256);
  log_debug("NEW ARCH KEY: %ld", a->hash_name);

  /* Init indexers and component containers */
  hash_to_size_inita(&a->offsets, w->allocator, TO_HEAP, 16);
  id_to_int64_inita(&a->entt_positions, w->allocator, TO_HEAP, 16);

  /* Init system cache */
  system_vec_inita(&a->contenders, w->allocator, TO_HEAP, 16);

  /* Init buffers */
  id_vec_inita(&a->entt_creation_buffer, w->allocator, TO_HEAP, 16);
  id_vec_inita(&a->entt_deletion_buffer, w->allocator, TO_HEAP, 16);
  id_vec_inita(&a->entt_mutation_buffer, w->allocator, TO_HEAP, 16);
  int64_vec_inita(&a->dead_fragment_buffer, w->allocator, TO_HEAP, 16);

  /* Apply type set */
  vec_to_set(key, &a->types);

  /* Construct the offset table. Increment and collect the sizes of each type */
  gsize component_pos = 0;
  void *args[3];
  args[0] = a;
  args[1] = w;
  args[2] = &component_pos;

  map_foreach(&a->types.internals, add_new_offset, args);

  /* The lenghts of an element in the composite vector is equal to the final
     position of 'component_pos' */
  __vec_init(&a->components, component_pos, w->allocator, TO_HEAP, 16);

  /* Caches don't get caches! Recursion base case here */
  if (SELECT_MODE(atomic_load(&w->id_gen)) == CACHED) {
    a->simulation = NULL;
    log_leave;
    return;
  }

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
  log_leave;
}

void free_archetype(archetype *a) {
  log_enter;
  assert(a);

  type_set_free(&a->types);
  vec_free(&a->components);
  hash_to_size_free(&a->offsets);
  id_to_int64_free(&a->entt_positions);
  stalloc_free(a->allocator);

  system_vec_free(&a->contenders);

  id_vec_free(&a->entt_creation_buffer);
  id_vec_free(&a->entt_deletion_buffer);
  id_vec_free(&a->entt_mutation_buffer);
  int64_vec_free(&a->dead_fragment_buffer);

  if (a->simulation) g_destroy_world(a->simulation);
  log_leave;
};

static compare(sort_hashes, int64_t, a, b, { return a < b; });
void archetype_key(char *types, hash_vec *hashes) {
  log_enter;
  assert(types);

  /* Count how many types there are to not waste stack space on reallocs */
  char   *origin = types;
  int64_t type_count = 1; /* 1 because we assert types is not zero. */
  while (*types) {
    if (*types == ',') type_count++;
    types++;
  }
  types = origin;

  hash_vec_sinit(hashes, type_count);

  int64_t end_pos, start_pos;
  origin = types;
  end_pos = 0;
  start_pos = 0;
  while (*types) {

    /* Skip spaces */
    while (*types == ' ') {
      start_pos++;
      end_pos++;
      types++;
    }

    if (*types == ',') {
      gid comp_id = (gid)hash_bytes(origin + start_pos, end_pos - start_pos);
      hash_vec_push(hashes, &comp_id);
      start_pos = end_pos + 1;
    }
    types++;
    end_pos++;
  }

  if (start_pos != end_pos) {
    gid comp_id = (gid)hash_bytes(origin + start_pos, end_pos - start_pos);
    hash_vec_push(hashes, &comp_id);
  }

  hash_vec_sort(hashes, sort_hashes, NULL);

  return;

  regex_t     matcher;
  regmatch_t *groups = stpush(sizeof(regmatch_t) * (type_count + 1));
  regcomp(&matcher, "(\\w+)", REG_EXTENDED);

  assert(regexec(&matcher, types, type_count + 1, groups, 0) != REG_NOMATCH);

  char *cursor = types;
  while (regexec(&matcher, cursor, type_count + 1, groups, 0) == 0) {
    regoff_t arg_start = groups[0].rm_so;
    regoff_t arg_end = groups[0].rm_eo;

    if (arg_start == -1 || arg_end == -1) break;

    gid comp_id = (gid)hash_bytes(&cursor[arg_start], arg_end - arg_start);

    /* We sort after all are pushed */
    hash_vec_push(hashes, &comp_id);

    cursor += arg_end; // Move the cursor to the end of the last match
  }
  hash_vec_sort(hashes, sort_hashes, NULL);
  log_leave;
}

feach(migrate_segments, kvpair, item, {
  void     **list = (void **)args;
  void      *prev_seg = list[0];
  void      *next_seg = list[1];
  g_core    *w = list[2];
  archetype *prev = list[3];
  archetype *next = list[4];

  uint64_t *type = item.key;

  log_debug("adding type: %ld", *type);

  gsize *component_size = hash_to_size_get(&w->component_registry, type).value;
  gsize *prev_offset = hash_to_size_get(&prev->offsets, type).value;
  gsize *next_offset = hash_to_size_get(&next->offsets, type).value;

  memmove(next_seg + *next_offset, prev_seg + *prev_offset, *component_size);
});
void delta_transition(g_core *w, gid entt, hash_vec *to_key) {
  log_enter;
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
    hash_to_archetype_put(&w->archetype_registry, &arch_id, &a);
    w->invalidate_fsm = 1;
  }

  /* Load the archetype of the current entity */
  kv = id_to_hash_get(&w->entity_registry, &entt);
  assert(kv.value && "Invalid Entity ID!");

  /* Load the archetype to transition to */
  a_next = hash_to_archetype_get(&w->archetype_registry, &arch_id).value;
  a_prev = load_entity_archetype(w, entt);

  /* Case 1: a_prev is the empty archetype. We don't need to do anything other
             than move the entity to a_next. No copying is necessary. */
  if (a_prev == &empty_archetype) {
    /* Add one more space for the incomming entity to this archetype. */
    int64_t pos = a_next->components.length;
    vec_resize(&a_next->components, a_next->components.length + 1);
    memset(composite_at(&a_next->components, pos), 0,
           a_next->components.__el_size);

    /* Add the position to entity map for easy id lookup */
    id_to_int64_put(&a_next->entt_positions, &entt, &pos);

    /* Update the world entity registry for global entity access */
    // TODO: make it so that put does overwrites so we dont need to delete
    id_to_hash_del(&w->entity_registry, &entt);
    id_to_hash_put(&w->entity_registry, &entt, &arch_id);
    log_leave;
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
  int32_t  old_flags = a_prev->types.internals.flags;
  a_prev->types.internals.flags = TO_STACK;
  type_set_intersect(&a_prev->types, &a_next->types, &retained_types);
  a_prev->types.internals.flags = old_flags;

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
  log_leave;
}