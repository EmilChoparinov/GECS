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
#include "ledger.h"   /* Contains the ledger implementation for concurrent 
                         contexts. */
#include "gid.h"      /* GECS smart IDs implementation. */
#include "register.h" /* Registers GECS types. */
#include "retcodes.h" /* Contains return code information. */
/*-------------------------------------------------------
 * Utilities
 *-------------------------------------------------------*/
#include "debug.h"  /* Adds GECS debugging features. */
#include "logger.h" /* Adds Logging. */

/*-------------------------------------------------------
 * Core Functions
 *-------------------------------------------------------*/
g_core_t *g_create_world(void);
retcode   g_destroy_world(g_core_t *w);
retcode   g_progress(g_core_t *w);

#define G_COMPONENT(w, ty) g_register_component(w, #ty, sizeof(ty))
retcode g_register_component(g_core_t *w, char *name, size_t component_size);

/* Note: g_add_component is not thread safe and is intended to be used only in
         the setup context (outside of systems) */
#define G_ADD_COMPONENT(w, id, ty) g_add_component(w, id, #ty)
retcode g_add_component(g_core_t *w, gid entt_id, char *name);

#define G_GET_COMPONENT(w, id, ty) (ty *)(g_get_component(w, id, #ty));
void *g_get_component(g_core_t *w, gid entt_id, char *name);

#define G_SET_COMPONENT(w, id, ty, ...)                                        \
  g_set_component(w, id, #ty, (void *)&(ty)__VA_ARGS__)
retcode g_set_component(g_core_t *w, gid entt_id, char *name, void *comp);

#define G_REM_COMPONENT(w, id, ty) g_rem_component(w, id, #ty)
retcode g_rem_component(g_core_t *w, gid entt_id, char *name);

#define G_SYSTEM(world, sys, ...) g_register_system(world, sys, #__VA_ARGS__)
retcode g_register_system(g_core_t *w, g_system sys, char *query);

gid g_create_entity(g_core_t *w);
gid gq_create_entity(g_query_t *q);

retcode g_queue_delete(g_core_t *w, gid entt);
retcode gq_queue_delete_entity(g_query_t *q, gid entt);

#define gq_add(q, id, ty) __gq_add(q, id, #ty)
retcode __gq_add(g_query_t *q, gid id, char *name);

#define gq_get(q, id, ty) (ty *)(__gq_get(q, id, #ty))
void *__gq_get(g_query_t *q, gid id, char *name);

#define gq_set(q, id, ty, ...) __gq_set(q, id, #ty, (void *)&(ty)__VA_ARGS__)
retcode __gq_set(g_query_t *q, gid entt_id, char *name, void *comp);

#define gq_rem(q, id, ty) __gq_rem(q, id, #ty)
retcode __gq_rem(g_query_t *q, gid entt_id, char *name);

#define gq_collect(q, ...) __gq_collect(q, #__VA_ARGS__)
g_itr __gq_collect(g_query_t *q, char *query);

#endif