#include "gecs.h"
#include "archetype.h"
#include "gid.h"

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

/*-------------------------------------------------------
 * Thread Safe Entity Operations
 *-------------------------------------------------------*/
/* Create an empty entity */
gid gq_create_entity(g_query *q);

/* Add an entity `entt` to the delete queue */
void gq_mark_delete(g_query *q, gid entt);

/* Check if a given entity `id` is currently processable by this system. */
bool gq_id_in(g_query *q, gid id);

