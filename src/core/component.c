#include "component.h"
#include "archetype.h"
#include "entity.h"
#include "gecs.h"

/*-------------------------------------------------------
 * Static Component Functions
 *-------------------------------------------------------*/
static g_core *select_component_location_ctx(g_query *q, gid entt, gid type) {
  /* Check if this archetype from the real context contains this component and
     return. If not, fall back to the simulation context and try there. */
  g_core *ctx = q->world_ctx;

  if (!type_set_has(&q->archetype_ctx->types, &type)) {
    /* Component must exist in the simulation context */
    ctx = q->archetype_ctx->simulation;
    archetype *sim_arch = load_entity_archetype(ctx, entt);
    assert(type_set_has(&sim_arch->types, &type) &&
           "Component does not exist on this entity!");
  }

  return ctx;
}

/*-------------------------------------------------------
 * Thread Unsafe Internal Component Operations
 *-------------------------------------------------------*/
feach(push_to_types, kvpair, item, {
  hash_vec *type_list = args;
  gid      *id = item.key;
  hash_vec_push(type_list, id);
});
void _g_add_component(g_core *w, gid entt, hash_vec *type_list) {
  log_enter;
  start_frame(w->allocator);

  archetype *entt_arch = load_entity_archetype(w, entt);

  /* Assert all components in new_types are unique. We only do this check if
     we are not empty */
  if (entt_arch != &empty_archetype) {
    for (int64_t i = 0; i < type_list->length; i++) {
      gid *component_id = hash_vec_at(type_list, i);
      assert(!type_set_has(&entt_arch->types, component_id) &&
             "Adding a component that already is on this entity!");
    }
  }

  /* Perform delta transition */
  if (entt_arch != &empty_archetype) {
    map_foreach(&entt_arch->types.internals, push_to_types, type_list);
  }
  delta_transition(w, entt, type_list);

  end_frame(w->allocator);
  log_leave;
}

void *_g_get_component(g_core *w, gid entt, gid type) {
  log_enter;
  start_frame(w->allocator);

  /* Load the archetype the entity exists in */
  archetype *entt_archetype = load_entity_archetype(w, entt);

  /* Load the position of the entities components in the composite. No assert is
     needed because load_entity_archetype will assert entt for us. */
  int64_t *entt_pos =
      id_to_int64_get(&entt_archetype->entt_positions, &entt).value;

  /* Load the position of the component in the composite */
  gsize *offset = hash_to_size_get(&entt_archetype->offsets, &type).value;
  assert(offset && "Given type does not exist on this archetype!");

  /* Return the pointer to the spot the component exists in */
  cbuff buff;
  buff_init(&buff, vec_at(&entt_archetype->components, *entt_pos));

  end_frame(w->allocator);
  log_leave;
  return buff_skip(&buff, *offset);
}

void _g_set_component(g_core *w, gid entt, gid type, void *comp_data) {
  log_enter;
  start_frame(w->allocator);

  /* Load the archetype the entity exists in */
  archetype *entt_archetype = load_entity_archetype(w, entt);

  /* Load the position of the entities components in the composite */
  int64_t *entt_pos =
      id_to_int64_get(&entt_archetype->entt_positions, &entt).value;

  /* Load the position of the component in the composite */
  gsize *offset = hash_to_size_get(&entt_archetype->offsets, &type).value;
  gsize *comp_size = hash_to_size_get(&w->component_registry, &type).value;
  assert(offset && "Given type does not exist on this archetype!");

  /* Get the address of the component within the composite and overwrite */
  cbuff buff;
  buff_init(&buff, vec_at(&entt_archetype->components, *entt_pos));
  memmove(buff_skip(&buff, *offset), comp_data, *comp_size);

  end_frame(w->allocator);
  log_leave;
}

/*-------------------------------------------------------
 * Thread Unsafe Component Operations
 *-------------------------------------------------------*/
void g_add_component(g_core *w, gid entt, char *new_types) {
  log_enter;
  start_frame(w->allocator);
  hash_vec types;
  archetype_key(new_types, &types);
  _g_add_component(w, entt, &types);
  end_frame(w->allocator);
  log_leave;
}

void *g_get_component(g_core *w, gid entt_id, char *name) {
  log_enter;
  log_leave;
  return _g_get_component(w, entt_id, (gid)hash_bytes(name, strlen(name)));
}

void g_set_component(g_core *w, gid entt, char *name, void *comp) {
  log_enter;
  _g_set_component(w, entt, (gid)hash_bytes(name, strlen(name)), comp);
  log_leave;
}

void g_rem_component(g_core *w, gid entt, char *remove_types) {
  log_enter;
  start_frame(w->allocator);

  /* Load the current entities archetype */
  archetype *entt_archetype = load_entity_archetype(w, entt);

  hash_vec rem_types;
  archetype_key(remove_types, &rem_types);

  /* Remove all types in rem_types from a copy of the entities type_set */
  type_set retained_types;
  type_set_copy(&entt_archetype->types, &retained_types);

  for (int64_t i = 0; i < rem_types.length; i++) {
    gid *comp_id = hash_vec_at(&rem_types, i);
    assert(type_set_has(&entt_archetype->types, comp_id) &&
           "Remove contains component already not on entity!");

    type_set_del(&retained_types, comp_id);
  }

  // TODO: I do not like this. Fix it
  hash_vec transition_typelist;
  set_to_vec((set *)&retained_types, (vec *)&transition_typelist);

  delta_transition(w, entt, &transition_typelist);

  end_frame(w->allocator);
  log_leave;
}

/*-------------------------------------------------------
 * Thread Safe Component Operations
 *-------------------------------------------------------*/
void __gq_add(g_query *q, gid entt, char *name) {
  /* Regardless of where the component exists. This transition must be
     simulated because it is not possible to parallelize. */
  id_vec_push(&q->archetype_ctx->entt_mutation_buffer, &entt);
  return g_add_component(q->archetype_ctx->simulation, entt, name);
}

void *__gq_get(g_query *q, gid entt, char *name) {
  gid type_id = (gid)hash_bytes(name, strlen(name));
  return g_get_component(select_component_location_ctx(q, entt, type_id), entt,
                         name);
}

void __gq_set(g_query *q, gid entt, char *name, void *comp) {
  gid type_id = (gid)hash_bytes(name, strlen(name));
  return g_set_component(select_component_location_ctx(q, entt, type_id), entt,
                         name, comp);
}

void __gq_rem(g_query *q, gid entt, char *name) {
  /* Very tricky. Component removal makes a mess of things but nothing that
     cannot be recovered from in the migration step.

     There are two cases: our first time deleting on this entity or future
     deletion.

     First time case: For the first time, we transition ALL components EXCEPT
     for the one specified for deletion. On the get/set steps, we always prefer
     the components inside of the real context over simulation. This is
     because it retains the vectorizability property.

     Future cases: For future times, when the entity gets their component
     deleted, we only need to transition the archetype in the simulation
     context. */

  archetype *arch = q->archetype_ctx;
  archetype *sim_arch = load_entity_archetype(arch->simulation, entt);
  id_vec_push(&q->archetype_ctx->entt_mutation_buffer, &entt);

  /* We check if this is the first time transitioning by seeing if the types
     intersect. If they do intersect then this is not the first time deleting */
  set inter_set;
  set_intersect(&arch->types, &sim_arch->types, &inter_set);
  if (set_length(&inter_set) > 0) {
    return g_rem_component(arch->simulation, entt, name);
  }

  /* Transition into simulation only. NOTE: we take the union because we want to
     retain whatever types are already in there. */
  type_set transition_to;
  type_set_union(&arch->types, &sim_arch->types, &transition_to);

  hash_vec key;
  set_to_vec(&transition_to, &key);
  delta_transition(q->archetype_ctx->simulation, entt, &key);

  return g_rem_component(q->archetype_ctx->simulation, entt, name);
}

int64_t gq_tick(g_query *q) { return q->world_ctx->tick; }
int64_t gq_tick_from_par(g_par par) { return par.tick; }
int64_t gq_tick_from_pool(g_pool pool) { return pool.entities.tick; }