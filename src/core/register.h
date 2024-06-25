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
#include <stdlib.h>

#include "map.h"
#include "set.h"
#include "vector.h"

/*-------------------------------------------------------
 * GECS Types
 *-------------------------------------------------------*/

/* All entities and archetypes share this same type for ids. The first 32 bits
   represents the real id of the entity. The last 32 bits represent various
   flags for the entity. All but the remaining last bits are reserved to the
   user to create tags. The last bit is reserved to flag the mode of the id.
   Cached or non-cached.  */
typedef uint64_t gid;

/* A blocked 256 char type. Used mainly for interfacing component names
   between the C macro expansions of the library. This is needed because the
   map implementation. If you have a component name longer than 256 bytes you
   need help. */
typedef char gstr[256];

/* Type representing the amount of bytes in memory some object is. */
typedef size_t gsize;

typedef int64_t gint;


/* Type representing the iteration structure used to concurrently modify
   components. */
typedef struct g_itr g_itr;

/* Since the composite vector knows nothing about the arrangement of data
   inside each element. Its effectively the any vector. The fragments part is
   for flavor for the user. */
typedef any fragment;

/* Type representing the iteration structure used to concurrently modify
   components. */
typedef struct g_itr g_itr;
typedef struct g_vec g_vec;

/* Type representing the interface given to each system. */
typedef struct g_query_t g_query_t;

/* The system type. */
typedef void (*g_system)(g_query_t *q);

/* Houses all structures related to the GECS library. Is returned when
   instancing a new gecs world. */
typedef struct g_core_t g_core_t;

/* Houses a single archetype structure. */
typedef struct archetype archetype;

/*-------------------------------------------------------
 * Vector Types
 *-------------------------------------------------------*/
VECTOR_GEN_H(bool)
VECTOR_GEN_H(gid);
VECTOR_GEN_H(gint);
VECTOR_GEN_H(fragment);

/*-------------------------------------------------------
 * Map Types
 *-------------------------------------------------------*/
MAP_GEN_H(gid, gsize);
MAP_GEN_H(gid, gid);
MAP_GEN_H(gstr, gint);

/*-------------------------------------------------------
 * Set Types
 *-------------------------------------------------------*/

#endif
