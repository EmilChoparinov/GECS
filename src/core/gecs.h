/* =========================================================================
    Author: E.D Choparinov, Amsterdam
    Related Files: gecs.h gecs.c
    Created On: June 10 2024
    Purpose:
        Entrypoint to the GECS Framework. Include this to include the
        library in your project.
========================================================================= */

#ifndef __HEADER_GECS_H__
#define __HEADER_GECS_H__

/* Support only up to 256 component compositions. */
#define GECS_MAX_ARCHETYPE_COMPOSITION 256

#include <assert.h>
#include <regex.h>
#include <stdarg.h>
#include <str_utils.h>

/*-------------------------------------------------------
 * GECS Core
 *-------------------------------------------------------*/
#include "gid.h"      /* GECS smart IDs implementation. */
#include "register.h" /* Registers GECS types. */
#include "retcodes.h" /* Contains return code information. */

/*-------------------------------------------------------
 * Utilities
 *-------------------------------------------------------*/
#include "buffer.h" /* Adds API for manipulating memory regions. */
#include "debug.h"  /* Adds GECS debugging features. */
#include "logger.h" /* Adds Logging. */

/*-------------------------------------------------------
 * World Functions
 *-------------------------------------------------------*/

/* Allocate a new GECS instance. Destroy with `g_destroy_world()` */
g_core_t *g_create_world(void);

/* Complete one tick */
retcode g_progress(g_core_t *w);

/* Destroy a GECS instance. */
retcode g_destroy_world(g_core_t *w);

/*-------------------------------------------------------
 * Thread Unsafe ECS Functions
 *-------------------------------------------------------*/

/* Unsafe: Register a component to the world. Ideally do this all at once in
           the beginning. */
#define G_COMPONENT(w, ty) g_register_component(w, #ty, sizeof(ty))
retcode g_register_component(g_core_t *w, char *name, size_t component_size);

/* Unsafe: Register a system to the world. Ideally do this all at once in the
           beginning. */
#define G_SYSTEM(world, sys, ...) g_register_system(world, sys, #__VA_ARGS__)
retcode g_register_system(g_core_t *w, g_system sys, char *query);

/* Unsafe: Create an empty entity. Use this only outside of the `g_progress`
           context. */
gid g_create_entity(g_core_t *w);

/* Unsafe: Add a new component with this function outside of the `g_progress`
           context. */
#define G_ADD_COMPONENT(w, id, ty) g_add_component(w, id, #ty)
retcode g_add_component(g_core_t *w, gid entt_id, char *name);

/* Unsafe: Get a component with this function outside of the `g_progres`
           context. */
#define G_GET_COMPONENT(w, id, ty) (ty *)(g_get_component(w, id, #ty));
void *g_get_component(g_core_t *w, gid entt_id, char *name);

/* Unsafe: Set a component with this function outside of the `g_progress`
           context */
#define G_SET_COMPONENT(w, id, ty, ...)                                        \
  g_set_component(w, id, #ty, (void *)&(ty)__VA_ARGS__)
retcode g_set_component(g_core_t *w, gid entt_id, char *name, void *comp);

/* Unsafe: Remove a component with this function outside of the `g_progress`
           context. */
#define G_REM_COMPONENT(w, id, ty) g_rem_component(w, id, #ty)
retcode g_rem_component(g_core_t *w, gid entt_id, char *name);

/* Unsafe: Add an entity `entt` to the delete queue in `w`. */
retcode g_queue_delete(g_core_t *w, gid entt);

/*-------------------------------------------------------
 * Thread Safe ECS Functions
 *-------------------------------------------------------*/

/* Create an empty entity */
gid gq_create_entity(g_query_t *q);

/* Add an entity `entt` to the delete queue */
retcode gq_queue_delete(g_query_t *q, gid entt);

/* Check if a given entity `id` is currently processable by this system. */
bool gq_id_in(g_query_t *q, gid id);

/* Add to entt `id` the component `ty` */
#define gq_add(q, id, ty) __gq_add(q, id, #ty)
retcode __gq_add(g_query_t *q, gid id, char *name);

/* Get a component `ty` of entity `entt`  */
#define gq_get(q, id, ty) (ty *)(__gq_get(q, id, #ty))
void *__gq_get(g_query_t *q, gid id, char *name);

/* Set a component `ty` of entity `id` with the struct given */
#define gq_set(q, id, ty, ...) __gq_set(q, id, #ty, (void *)&(ty)__VA_ARGS__)
retcode __gq_set(g_query_t *q, gid entt_id, char *name, void *comp);

/* Remove a component `ty` from entity `id` */
#define gq_rem(q, id, ty) __gq_rem(q, id, #ty)
retcode __gq_rem(g_query_t *q, gid entt_id, char *name);

/*-------------------------------------------------------
 * System Query Functions
 *-------------------------------------------------------*/

/* Convert query `q` into a vector to process in parallel. */
g_vec gq_vectorize(g_query_t *q);

/* Convert query `q` into an iterator to process however you need.  */
g_itr gq_seq(g_query_t *q);

/* Go to the next entity in the fragment. */
g_itr gq_next(g_itr itr);

/* Check if the iterator is finished. */
bool gq_done(g_itr itr);

/* Process vectorized tasks on each entity existing in the vector. */
#define gq_each(vec, func, args) __gq_each(vec, (f_each)func, (void *)args);
void __gq_each(g_vec vec, f_each func, void *args);

/* Select a component from the iterator to manipulate manually. */
#define gq_field(itr, ty) (ty *)(__gq_field(itr, #ty))
void *__gq_field(g_itr *itr, char *type);

/* Select the composite element and for low level manipulations of the
   component. */
#define gq_take(q, frag, ty) (ty *)(__gq_take(q, frag, #ty))
fragment *__gq_take(g_query_t *q, fragment *frag, char *name);

/* Select the fragment at a given component index. NOTE: This index is not
   the entity id. There exists a map entity_registry that contains record
   of the index to load this with. */
#define gq_index(q, i, ty) (ty *)(__gq_index(q, i, #ty))
fragment *__gq_index(g_query_t *q, gint index, char *name);

#endif