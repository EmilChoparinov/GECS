/* =========================================================================
    Author: E.D Choparinov, Amsterdam
    Related Files: gecs_register.h gecs_register.c
    Created On: June 01 2024
    Purpose:
        The purpose of this file is to register autogenerating structures
        to be used in other parts of the library.
========================================================================= */

#ifndef __HEADER_REGISTER_H__
#define __HEADER_REGISTER_H__

#include <stdint.h>

/*-------------------------------------------------------
 * GECS Types
 *-------------------------------------------------------*/

/* All entites and archetypes share this same type for ids. Its int64_t not
   uint64_t because I don't want to deal with unsignedness. */
typedef int64_t gid;

/* A blocked 256 char type. Used mainly for interfacing component names
   between the C macro expansions of the library. This is needed because the
   map implementation. If you have a component name longer than 256 bytes you
   need help. */
typedef char gstr[256];

/* Type representing the amount of bytes in memory some object is. Same reason
   as gid as to why its int64_t. */
typedef int64_t gsize;

typedef int64_t gint;

/* Type representing the interface given to each system. */
typedef struct g_query_t g_query_t;

/* The system type. */
typedef void (*g_system)(g_query_t *q);

/* Houses all structures related to the GECS library. Is returned when
   instancing a new gecs world. */
typedef struct g_core_t g_core_t;

#include "vector.h"

/*-------------------------------------------------------
 * Vector Types
 *-------------------------------------------------------*/
VECTOR_GEN_H(bool)
VECTOR_GEN_H(gid);

#include "map.h"

/*-------------------------------------------------------
 * Map Types
 *-------------------------------------------------------*/
MAP_GEN_H(gid, gsize);
MAP_GEN_H(gid, gid);
MAP_GEN_H(gstr, gint);

#include "set.h"

/*-------------------------------------------------------
 * Set Types
 *-------------------------------------------------------*/

#endif
