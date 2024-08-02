/* =========================================================================
    Author: E.D Choparinov, Amsterdam
    Related Files: gecs.h gecs.c
    Created On: July 09 2024
    Purpose:
        The purpose of this file is to house the GECS library implementation
        API. The types used in this file are located in 'types.h' and the
        datastructures used are derived from 'libcsdsa'.
========================================================================= */
#ifndef __HEADER_GECS_H__
#define __HEADER_GECS_H__

#include "logger.h"
#include "types.h"
#include <stdatomic.h>

/*-------------------------------------------------------
 * GECS Library Variables
 *     *_START: Define the initial sizes of objects
 *              existing in g_core.
 *-------------------------------------------------------*/
#define ARCHETYPE_REG_START 16
#define COMPONENT_REG_START 16
#define ENTITY_REG_START    16
#define SYSTEM_REG_START    16

#define SYS_READONLY 1
#define DEFAULT      0

struct g_core {
  int64_t tick; /* The amount of times the world has progressed. */
  atomic_uint_least64_t id_gen; /* Generates unique IDs. This is typed as least
                                   and not fast uint64_t by design because of
                                   the papers smart entity bit arithmetic. */
  /* Flags:
      `invalidate_fsm` - When set to 1, it will reprocess the FSM graph before
                      next tick.
      `is_main` - When set to 1, it represents this world is the sequential
                one in the graph to prevent infinite recursion.
      `disable_concurrency` - Deny the ECS from making and using threads. */
  int8_t invalidate_fsm, is_sequential, disable_concurrency;

  stalloc *allocator; /* Internal stack allocations done here. */

  /* Map : hash(Ordered[comp name]) -> archetype */
  hash_to_archetype archetype_registry;
  hash_to_size      component_registry; /* Map : hash(comp name) -> comp size */
  id_to_hash entity_registry; /* Map : entt id -> hash(Ordered[comp name]) */
  system_vec system_registry; /* Vec : system_data */
};

typedef struct GecID GecID;
struct GecID {
  gid id;
};

/*-------------------------------------------------------
 * Container Operations
 *-------------------------------------------------------*/
/* Allocate a new GECS instance. Destroy with `g_destroy_world` */
g_core *g_create_world(void);

/* Process one tick */
void g_progress(g_core *w);

/* Destroy a GECS instance. */
void g_destroy_world(g_core *w);

/*-------------------------------------------------------
 * Thread Unsafe Registration Operations
 *-------------------------------------------------------*/
/* Unsafe: Register a component to the world. Ideally do this all at once in
           the beginning. */
#define G_COMPONENT(w, ty) g_register_component(w, #ty, sizeof(ty))
void g_register_component(g_core *w, char *name, size_t component_size);

/* Unsafe: Register a system to the world. Ideally do this all at once in the
           beginning. */
#define G_SYSTEM(world, sys, FLAGS, ...)                                       \
  g_register_system(world, sys, FLAGS, #__VA_ARGS__)
void g_register_system(g_core *w, g_system sys, int32_t FLAGS, char *query);

/*-------------------------------------------------------
 * Thread Unsafe Entity Operations
 *-------------------------------------------------------*/
/* Unsafe: Create an empty entity. Use this only outside of the `g_progress`
           context. */
gid g_create_entity(g_core *w);

/* Unsafe: Add an entity `entt` to the delete queue in `w`. */
void g_mark_delete(g_core *w, gid entt);

/*-------------------------------------------------------
 * Thread Safe Entity Operations
 *-------------------------------------------------------*/
/* Create an empty entity */
gid gq_create_entity(g_query *q);

/* Add an entity `entt` to the delete queue */
void gq_mark_delete(g_query *q, gid entt);

/* Check if a given entity `id` is currently processable by this system. */
bool gq_id_in(g_query *q, gid id);

bool gq_id_alive(g_query *q, gid id);

#define gq_field_by_id(q, entt, ty) (ty *)(__gq_field_by_id(q, entt, #ty))
void *__gq_field_by_id(g_query *q, gid entt, char *type);

/*-------------------------------------------------------
 * Thread Unsafe Component Operations
 *-------------------------------------------------------*/
/* Unsafe: Add a new component with this function outside of the `g_progress`
           context. */
#define G_ADD_COMPONENT(w, id, ty) g_add_component(w, id, #ty)
void g_add_component(g_core *w, gid entt, char *name);

/* Unsafe: Get a component with this function outside of the `g_progres`
           context. */
#define G_GET_COMPONENT(w, id, ty) (ty *)(g_get_component(w, id, #ty));
void *g_get_component(g_core *w, gid entt, char *name);

/* Unsafe: Set a component with this function outside of the `g_progress`
           context */
#define G_SET_COMPONENT(w, id, ty, ...)                                        \
  g_set_component(w, id, #ty, (void *)&(ty)__VA_ARGS__)
void g_set_component(g_core *w, gid entt, char *name, void *comp);

/* Unsafe: Remove a component with this function outside of the `g_progress`
           context. */
#define G_REM_COMPONENT(w, id, ty) g_rem_component(w, id, #ty)
void g_rem_component(g_core *w, gid entt, char *name);

/* Unsafe: Check if a component exists. Use function outside of the `g_progress`
           context. */
#define G_HAS_COMPONENT(w, id, ty) g_has_component(w, id, #ty)
bool g_has_component(g_core *w, gid entt, char *name);

/*-------------------------------------------------------
 * Thread Safe Component Operations
 *-------------------------------------------------------*/
/* Add to entt `id` the component `ty` */
#define gq_add(q, id, ty) __gq_add(q, id, #ty)
void __gq_add(g_query *q, gid id, char *name);

#define gq_mut(q, id) __gq_mut(q, id);
void __gq_mut(g_query *q, gid entt);

/* Get a component `ty` of entity `entt`  */
#define gq_get(q, id, ty) (ty *)(__gq_get(q, id, #ty))
void *__gq_get(g_query *q, gid id, char *name);

/* Set a component `ty` of entity `id` with the struct given */
#define gq_set(q, id, ty, ...) __gq_set(q, id, #ty, (void *)&(ty)__VA_ARGS__)
void __gq_set(g_query *q, gid entt, char *name, void *comp);

/* Remove a component `ty` from entity `id` */
#define gq_rem(q, id, ty) __gq_rem(q, id, #ty)
void __gq_rem(g_query *q, gid entt, char *name);

#define gq_has(q, id, ty) __gq_has(q, id, #ty)
bool __gq_has(g_query *q, gid entt, char *name);

int64_t gq_tick(g_query *q);
int64_t gq_tick_from_par(g_par par);
int64_t gq_tick_from_pool(g_pool pool);

/*-------------------------------------------------------
 * Tag Operations
 *-------------------------------------------------------*/
/* Generate new tag for GECS to use */
#define TAG(ty)                                                                \
  typedef struct ty ty;                                                        \
  struct ty {                                                                  \
    int8_t _;                                                                  \
  };

#define G_TAG(w, ty) G_COMPONENT(w, ty);

/*-------------------------------------------------------
 * Sequential Query Operations
 *-------------------------------------------------------*/
/* Convert query `q` into an iterator to process however you need.  */
g_pool gq_seq(g_query *q);

/* Go to the next entity in the fragment. */
g_pool gq_next(g_pool itr);

/* Check if the iterator is finished. */
bool gq_done(g_pool itr);

/* Select a component from the iterator to manipulate manually. */
#define gq_field(itr, ty) (ty *)(__gq_field(&itr, #ty))
void *__gq_field(g_pool *itr, char *type);

#define G_GET_POOL(world, ...) g_get_pool(world, "GecID, " #__VA_ARGS__)
g_pool g_get_pool(g_core *world, char *query);

/*-------------------------------------------------------
 * Parallel Query Operations
 *-------------------------------------------------------*/
/* Convert query `q` into a vector to process in parallel. */
g_par gq_vectorize(g_query *q);

/* Process vectorized tasks on each entity existing in the vector. */
#define gq_each(vec, func, args) __gq_each(vec, (_each)func, (void *)args);
void __gq_each(g_par vec, _each func, void *args);

#endif