#include "archetype.h"
#include "gecs.h"
#include "gid.h"

/*-------------------------------------------------------
 * Static Entity Functions
 *-------------------------------------------------------*/
static gid create_entity_using_idgen(g_core *w, atomic_uint_least64_t *idgen) {
  log_enter;

  gid id = gid_atomic_incr(idgen);

  /* All entities initially start at the empty archetype. This is simulated
     as such: */
  id_to_hash_put(&w->entity_registry, &id, &empty_archetype.hash_name);

  log_leave;
  return id;
}

/*-------------------------------------------------------
 * Internal GECS Library Functions
 *-------------------------------------------------------*/
archetype *load_entity_archetype(g_core *w, gid entt) {
  log_enter;

  /* Load archtype hash name */
  gid *archetype_id = id_to_hash_get(&w->entity_registry, &entt).value;
  assert(archetype_id && "Entity does not exist!");

  /* Load archetype */
  archetype *arch =
      hash_to_archetype_get(&w->archetype_registry, archetype_id).value;
  if (!arch) return &empty_archetype;
  return arch;
  log_leave;
}

/*-------------------------------------------------------
 * Thread Unsafe Entity Operations
 *-------------------------------------------------------*/
gid g_create_entity(g_core *w) {
  log_enter;
  gid id = create_entity_using_idgen(w, &w->id_gen);
  G_ADD_COMPONENT(w, id, GecID);
  G_SET_COMPONENT(w, id, GecID, {.id = id});
  log_leave;
  return id;
}

void g_mark_delete(g_core *w, gid entt) {
  log_enter;

  archetype *arch = load_entity_archetype(w, entt);
  if (arch == &empty_archetype) {
    id_to_hash_del(&w->entity_registry, &entt);
    return;
  }

  id_to_hash_del(&w->entity_registry, &entt);
  id_to_hash_del(&arch->entt_positions, &entt);
  id_vec_push(&arch->entt_deletion_buffer, &entt);

  log_leave;
}

/*-------------------------------------------------------
 * Thread Safe Entity Operations
 *-------------------------------------------------------*/
/* Create an empty entity */
gid gq_create_entity(g_query *q) {
  log_enter;

  /* We use the idgen from world as the id gen in the simulation to keep the
     IDs consistent. This is ok because idgen is made to be thread safe. */
  gid id = create_entity_using_idgen(q->archetype_ctx->simulation,
                                     &q->world_ctx->id_gen);

  log_leave;
  return id;
}

/* Add an entity `entt` to the delete queue */
void gq_mark_delete(g_query *q, gid entt) {
  /* Find where entt exists. There are two positions it may live in: world or
     simulation */
  if (id_to_hash_has(&q->world_ctx->entity_registry, &entt)) {
    g_mark_delete(q->world_ctx, entt);
    return;
  }

  if (id_to_hash_has(&q->archetype_ctx->simulation->entity_registry, &entt)) {
    g_mark_delete(q->archetype_ctx->simulation, entt);
    return;
  }

  assert(false && "Entity marked to delete does not exist!");
}

/* Check if a given entity `id` is currently processable by this system. */
bool gq_id_in(g_query *q, gid id) {
  log_enter;

  /* Check if in the world, else check if in the simulation */
  return id_to_hash_has(&q->world_ctx->entity_registry, &id) ||
         id_to_hash_has(&q->archetype_ctx->simulation->entity_registry, &id);

  log_leave;
}

bool gq_id_alive(g_query *q, gid id) {
  return id_to_hash_has(&q->world_ctx->entity_registry, &id);
}

void *__gq_field_by_id(g_query *q, gid entt, char *type) {
  /* This is a guard to retain valid concurrency.  */
  int64_t *pos =
      id_to_int64_get(&q->archetype_ctx->entt_positions, &entt).value;
  assert(pos && "Entity does not exist on this archetype!");

  return g_get_component(q->world_ctx, entt, type);
}