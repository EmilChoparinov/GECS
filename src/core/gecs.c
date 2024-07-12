#include "gecs.h"
#include "archetype.h"
#include "gid.h"
#include "logger.h"

#include <stdio.h>

/*-------------------------------------------------------
 * Generated Types
 *-------------------------------------------------------*/
VEC_TYPE_IMPL(system_vec, system_data);

/*-------------------------------------------------------
 * Container Operations
 *-------------------------------------------------------*/
g_core *g_create_world(void) {
  log_enter;

  g_core *w = calloc(1, sizeof(*w));

  atomic_init(&w->id_gen, 0);
  gid_atomic_set(&w->id_gen, STORAGE);

  w->invalidate_fsm = 1;
  w->is_sequential = 1;

  w->allocator = stalloc_create(STALLOC_DEFAULT);

  hash_to_archetype_inita(&w->archetype_registry, w->allocator, TO_HEAP,
                          ARCHETYPE_REG_START);
  hash_to_size_inita(&w->component_registry, w->allocator, TO_HEAP,
                     COMPONENT_REG_START);
  id_to_hash_inita(&w->entity_registry, w->allocator, TO_HEAP,
                   ENTITY_REG_START);
  system_vec_inita(&w->system_registry, w->allocator, TO_HEAP,
                   SYSTEM_REG_START);

  /* I decided to not put the empty archetype into the registry because I do
     not want to risk adding an extra collision when querying. It's ok to not
     put it in because the empty archetype represents garbage, so a user
     cannot query for it. */
  log_leave;
  return w;
}

void g_progress(g_core *w) {
  log_enter;

  log_leave;
}

feach(f_free_archetype, kvpair, item, { free_archetype(item.value); });
feach(f_free_system, system_data, sys, { type_set_free(&sys.requirements); });
void g_destroy_world(g_core *w) {
  log_enter;

  hash_to_archetype_foreach(&w->archetype_registry, f_free_archetype, NULL);
  hash_to_archetype_free(&w->archetype_registry);

  system_vec_foreach(&w->system_registry, f_free_system, NULL);
  system_vec_free(&w->system_registry);

  id_to_size_free(&w->component_registry);
  id_to_hash_free(&w->entity_registry);

  stalloc_free(w->allocator);
  free(w);

  log_leave;
}

/*-------------------------------------------------------
 * Thread Unsafe Registration Operations
 *-------------------------------------------------------*/
void g_register_component(g_core *w, char *name, size_t component_size) {
  log_enter;
  start_frame(w->allocator);

  uint64_t hash_name = hash_bytes(name, strlen(name));

  assert(!hash_to_size_has(&w->component_registry, name) &&
         "Collision detection: name is either re-registered or another "
         "component contains the same hashname. Exiting");

  hash_to_size_put(&w->component_registry, &hash_name, &component_size);

  end_frame(w->allocator);
  log_leave;
}

static feach(add_to_set, uint64_t, hash, {
  type_set *types = args;
  type_set_put(types, &hash);
});
static feach(is_registered, uint64_t, hash, {
  g_core *w = args;
  assert(hash_to_size_get(&w->component_registry, &hash).value &&
         "Error: attempted to register a system with unregistered component "
         "types.");
});
void g_register_system(g_core *w, g_system sys, char *query) {
  log_enter;
  start_frame(w->allocator);

  /* Collect the component id hashes */
  hash_vec type_hashes;
  archetype_key(query, &type_hashes);
  hash_vec_foreach(&type_hashes, is_registered, w); /* Sanity check */

  type_set types;
  type_set_hinit(&types);
  hash_vec_foreach(&type_hashes, add_to_set, &types);

  system_vec_push(&w->system_registry,
                  &(system_data){.requirements = types, .start_system = sys});

  end_frame(w->allocator);
  log_leave;
}

/*-------------------------------------------------------
 * Thread Unsafe Entity Operations
 *-------------------------------------------------------*/
gid g_create_entity(g_core *w) {
  log_enter;
  gid id = gid_atomic_incr(&w->id_gen);

  /* All entities initially start at the empty archetype. This is simulated
     as such: */
  id_to_hash_put(&w->entity_registry, &id, &empty_archetype.hash_name);

  log_leave;
  return id;
}

void g_mark_delete(g_core *w, gid entt) {
  log_enter;

  /* Load archetype hash name from entity_registry */
  kvpair ret = id_to_hash_get(&w->entity_registry, &entt);
  assert(ret.value && "Entity does not exist!");
  uint64_t hash_name = *(uint64_t *)ret.value;
  if (!hash_name) { /* Empty archetype */
    id_to_hash_del(&w->entity_registry, &entt);
    return;
  }

  archetype *a = hash_to_archetype_get(&w->archetype_registry, ret.value).value;

  id_vec_push(&a->entt_deletion_buffer, &entt);

  log_leave;
}
