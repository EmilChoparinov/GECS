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

/* General GECS ID. The way entities use this ID is smarter than the way
   archetypes use it, but nonetheless use the same type. */
typedef uint64_t gid;

/* Type representing the amount of bytes in memory some object is. */
typedef size_t gsize;

/* General GECS integer type */
typedef int64_t gint;

/* Type representing the iteration structure used to concurrently modify
   components. */

/* Iteration structure used for sequential operations over fragments. */
typedef struct g_itr g_itr;

/* Iteration structure for concurrent processing vectorized entities. */
typedef struct g_vec g_vec;

/* Type for representing a function that performs one element operation
   per call. */
typedef void (*f_each)(g_itr *itr, void *);

/* A fragment is a set of interleaved components that exist on the composite
   vector. This is the element type of the composite. Since the composite
   vector knowns nothing about the arrangement of data inside each element,
   it is effectively an any vector. */
typedef any fragment;

/* Type representing the interface between user <-> GECS. */
typedef struct g_query_t g_query_t;

/* The system type. */
typedef void (*g_system)(g_query_t *q);

/* A GECS instance. */
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

#endif
