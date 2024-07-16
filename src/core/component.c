#include "component.h"
#include "archetype.h"
#include "gecs.h"

/*-------------------------------------------------------
 * Static Functions
 *-------------------------------------------------------*/
// static archetype *load_archetype_from_entity(g_core *w, gid entt) {
//   log_enter;

//   /* Assert entity exists */
//   gid *archetype_id = id_to_hash_get(&w->entity_registry, &entt).value;
//   assert(archetype_id && "Invalid Entity ID given!");

//   /* Load archetype */
//   log_leave_after(
//       hash_to_archetype_get(&w->archetype_registry, archetype_id).value);
//   //   log_leave;
//   //   return hash_to_archetype_get(&w->archetype_registry, archetype_id).value;
// }

/*-------------------------------------------------------
 * Thread Unsafe Internal Component Operations
 *-------------------------------------------------------*/
void _g_add_component(g_core *w, gid entt, hash_vec *new_types) {
  log_enter;
  start_frame(w->allocator);

  /* Assert entity exists */
  gid *archetype_id = id_to_hash_get(&w->entity_registry, &entt).value;
  assert(archetype_id && "Invalid Entity ID given");

  /* Load archetype */
  //   archetype *arch =
  //       hash_to_archetype_get(&w->archetype_registry, archetype_id).value;

  /* Assert all components in new_types are unique */
  for (int64_t i = 0; i < new_types->length; i++) {
    gid *component_id = hash_vec_at(new_types, i);
    assert(!hash_to_size_get(&w->component_registry, component_id).value &&
           "Adding a component that already is on this entity!");
  }

  /* Perform delta transition */
  delta_transition(w, entt, new_types);

  end_frame(w->allocator);
  log_leave;
}

void *_g_get_component(g_core *w, gid entt, gid type) {
  /* Assert entity exists */
  //   gid *archetype_id
  return NULL;
}

/*-------------------------------------------------------
 * Thread Unsafe Component Operations
 *-------------------------------------------------------*/
void g_add_component(g_core *w, gid entt, char *new_types) {
  log_enter;
  hash_vec types;
  archetype_key(new_types, &types);
  _g_add_component(w, entt, &types);
  log_leave;
}

void *g_get_component(g_core *w, gid entt_id, char *name) { return NULL; }

/*
static void *__g_get_component(g_core_t *w, gid entt_id, gid type) {
  gid_entity_record_map_item *item =
      gid_entity_record_map_find(&w->entity_registry, &entt_id);
  assert(item && "Entity does not exist!");

  entity_record entt_rec = item->value;

  gid_gsize_map_item *typekv = gid_gsize_map_find(&entt_rec.a->offsets, &type);
  assert(typekv && "Component not registered!");
  gsize comp_off = typekv->value;

  buff_t buff;
  buff_init(&buff, fragment_vec_at(&entt_rec.a->composite, entt_rec.index));

  return buff_skip(&buff, comp_off);
}

void *g_get_component(g_core_t *w, gid entt_id, char *name) {
  log_info("get component {id: %ld, name: %s, ctx:%d }\n", entt_id, name,
           SELECT_MODE(entt_id));
  return __g_get_component(w, entt_id, (gid)hash_bytes(name, strlen(name)));
}
*/

void g_set_component(g_core *w, gid entt_id, char *name, void *comp);

void g_rem_component(g_core *w, gid entt_id, char *name);

/*-------------------------------------------------------
 * Thread Safe Component Operations
 *-------------------------------------------------------*/
void __gq_add(g_query *q, gid id, char *name);

void *__gq_get(g_query *q, gid id, char *name);

void __gq_set(g_query *q, gid entt_id, char *name, void *comp);

void __gq_rem(g_query *q, gid entt_id, char *name);
