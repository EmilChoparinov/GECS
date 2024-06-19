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

#define G_COMPONENT(w, ty) g_register_component(w, #ty, sizeof(ty));
#define G_SYSTEM(world, sys, ...) g_register_system(world, sys, #__VA_ARGS__);

gid g_create_entity(g_core_t *w);

retcode g_register_component(g_core_t *w, char *name, size_t component_size);
retcode g_register_system(g_core_t *w, g_system *sys, char *query);

#endif