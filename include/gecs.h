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

#include "types.h"
#include <stdatomic.h>

/*-------------------------------------------------------
 * GECS Library Variables
 *     *_START: Define the initial sizes of objects
 *              existing in g_core.
 *-------------------------------------------------------*/
#define ARCHETYPE_REG_START 1
#define COMPONENT_REG_START 1
#define ENTITY_REG_START    1
#define SYSTEM_REG_START    1

typedef struct system_data system_data;
struct system_data {
  g_system start_system; /* A function pointer to a user defined function. */
  type_set requirements; /* Set : [hash(comp name)] */
};
VEC_TYPEDEC(system_vec, system_data);

struct g_core {
  atomic_uint_least64_t id_gen; /* Generates unique IDs. This is typed as least
                                   and not fast uint64_t by design because of
                                   the papers smart entity bit arithmetic. */
  /* Flags:
      `invalidate_fsm` - When set to 1, it will reprocess the FSM graph before
                      next tick.
      `is_main` - When set to 1, it represents this world is the sequential
                one in the graph to prevent infinite recursion. */
  int8_t invalidate_fsm, is_sequential;

  stalloc *allocator; /* Internal stack allocations done here. */

  /* Map : hash(Ordered[comp name]) -> archetype */
  hash_to_archetype archetype_registry;
  hash_to_size      component_registry; /* Map : hash(comp name) -> comp size */
  id_to_hash entity_registry; /* Map : entt id -> hash(Ordered[comp name]) */
  system_vec system_registry; /* Vec : system_data */
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
#define G_SYSTEM(world, sys, ...) g_register_system(world, sys, #__VA_ARGS__)
void g_register_system(g_core *w, g_system sys, char *query);

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

/*-------------------------------------------------------
 * Thread Unsafe Component Operations
 *-------------------------------------------------------*/
/* Unsafe: Add a new component with this function outside of the `g_progress`
           context. */
#define G_ADD_COMPONENT(w, id, ty) g_add_component(w, id, #ty)
void g_add_component(g_core *w, gid entt_id, char *name);

/* Unsafe: Get a component with this function outside of the `g_progres`
           context. */
#define G_GET_COMPONENT(w, id, ty) (ty *)(g_get_component(w, id, #ty));
void *g_get_component(g_core *w, gid entt_id, char *name);

/* Unsafe: Set a component with this function outside of the `g_progress`
           context */
#define G_SET_COMPONENT(w, id, ty, ...)                                        \
  g_set_component(w, id, #ty, (void *)&(ty)__VA_ARGS__)
void g_set_component(g_core *w, gid entt_id, char *name, void *comp);

/* Unsafe: Remove a component with this function outside of the `g_progress`
           context. */
#define G_REM_COMPONENT(w, id, ty) g_rem_component(w, id, #ty)
void g_rem_component(g_core *w, gid entt_id, char *name);

/*-------------------------------------------------------
 * Thread Safe Component Operations
 *-------------------------------------------------------*/
/* Add to entt `id` the component `ty` */
#define gq_add(q, id, ty) __gq_add(q, id, #ty)
void __gq_add(g_query *q, gid id, char *name);

/* Get a component `ty` of entity `entt`  */
#define gq_get(q, id, ty) (ty *)(__gq_get(q, id, #ty))
void *__gq_get(g_query *q, gid id, char *name);

/* Set a component `ty` of entity `id` with the struct given */
#define gq_set(q, id, ty, ...) __gq_set(q, id, #ty, (void *)&(ty)__VA_ARGS__)
void __gq_set(g_query *q, gid entt_id, char *name, void *comp);

/* Remove a component `ty` from entity `id` */
#define gq_rem(q, id, ty) __gq_rem(q, id, #ty)
void __gq_rem(g_query *q, gid entt_id, char *name);

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
#define gq_field(itr, ty) (ty *)(__gq_field(itr, #ty))
void *__gq_field(g_pool *itr, char *type);

/*-------------------------------------------------------
 * Parallel Query Operations
 *-------------------------------------------------------*/
/* Convert query `q` into a vector to process in parallel. */
g_par gq_vectorize(g_query *q);

/* Process vectorized tasks on each entity existing in the vector. */
#define gq_each(vec, func, args) __gq_each(vec, (f_each)func, (void *)args);
void __gq_each(g_par vec, _each func, void *args);

#endif