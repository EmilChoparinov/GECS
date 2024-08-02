/* =========================================================================
    Author: E.D Choparinov, Amsterdam
    Related Files: types.h types.c
    Created On: July 09 2024
    Purpose:
        The purpose of this file is to define, generate, or derive types
        used within the GECS library. Type generation is done via the
        'libcsdsa' library.
========================================================================= */
#ifndef __HEADER_TYPES_H__
#define __HEADER_TYPES_H__

#include "csdsa.h"

/*-------------------------------------------------------
 * Core Types
 *-------------------------------------------------------*/
/* General library identifier. Entities and Archetypes both use this value.
   Entities are smarter with it. Check the entity implementation for more
   details. */
typedef uint64_t gid;

/* Type representing the amount of bytes in memory some object has. */
typedef uint64_t gsize;

/* General GECS integer type. */
typedef int64_t gint64;

/* Iteration structure used for sequential operations over fragments. */
typedef struct g_pool g_pool;

/* Iteration structure used for concurrent processing over vectorized
   fragments. */
typedef struct g_par g_par;

/* Type representing the interface between a system and GECS */
typedef struct g_query g_query;

/* Type representing a system user function */
typedef void (*g_system)(g_query *q);

/* Type representing the system properties struct  */
typedef struct system_data system_data;

/* The core object GECS uses to manipulate its runtime. */
typedef struct g_core g_core;

/*-------------------------------------------------------
 * Generated Types
 *-------------------------------------------------------*/

/* A composite vector is a list of interleaved components such that each
   index pertains to one entities data. Since the composite vector knows
   nothing about the arrangement of data inside each element, initialization
   is done manually but the type is here for type help. */
VEC_TYPEDEC(composite, void *);

VEC_TYPEDEC(id_vec, gid);
VEC_TYPEDEC(int64_vec, int64_t);

MAP_TYPEDEC(id_to_size, gid, gsize);
MAP_TYPEDEC(id_to_id, gid, gid);
MAP_TYPEDEC(id_to_int64, gid, int64_t);
MAP_TYPEDEC(id_to_archetype, gid, archetype);

MAP_TYPEDEC(id_to_hash, gid, uint64_t);
MAP_TYPEDEC(hash_to_size, uint64_t, gsize);
VEC_TYPEDEC(hash_vec, uint64_t);
MAP_TYPEDEC(hash_to_archetype, uint64_t, archetype);

SET_TYPEDEC(type_set, int64_t);

/* Cache generated types. These vectors contain only address to other data
   found in the structures above or just addresses in general. */
VEC_TYPEDEC(cache_vec, void *);
MAP_TYPEDEC(cache_map, gid, void *);

VEC_TYPEDEC(system_vec, system_data);
/*-------------------------------------------------------
 * Public Structure Definitions
 *-------------------------------------------------------*/
typedef struct archetype archetype;
struct archetype {
  gid      archetype_id; /* Unique identifier for this archetype. */
  uint64_t hash_name;    /* This is the hash of hash(Ordered(types)) */

  stalloc *allocator; /* Allocator used in concurrent contexts */

  /* These two types are used for scheduling systems. Types is also used for
     transitioning archetypes. */
  type_set types; /* Set : [hash(comp name)] */

  g_core *simulation; /* Entity transition simulations are done here. */

  /* These three members are used for indexing and component retrieval. */
  composite    components; /* Contiguous vector of interleaved components. */
  hash_to_size offsets;    /* Map : hash(comp id) -> gsize */
  id_to_int64  entt_positions; /* Map : gid -> gint */

  /* The following members are made for concurrency and caching purposes. */

  /* A list of addresses pointing to system_data structs existing in the g_core
     struct. */
  system_vec contenders; /* Vec : system_data */

  /* entt_creation_buffer is filled when a system created new entities
     concurrently. */
  id_vec entt_creation_buffer; /* Vec : entt id */

  /* entt_deletion_buffer is filled when a system creates new entities
     concurrently. */
  id_vec entt_deletion_buffer; /* Vec : entt id */

  /* Entities that have simulated operations over a tick are stored here. */
  id_vec entt_mutation_buffer; /* Vec : entt id */

  /* dead_fragment_buffer is filled when a system transitions an entity off
     this archetype. These fragments are collected and cleaned per tick. */
  int64_vec dead_fragment_buffer; /* Vec : index_of(composite) */
};

struct g_par {
  composite    *stored_components; /* Vector : fragment */
  hash_to_size *component_offsets; /* Map : hash(comp id) -> gsize */
  archetype    *arch;
  g_core       *world;
  int64_t       tick;
};

struct g_pool {
  gint64 idx;
  g_par  entities; /* Vector : any size */
};

struct g_query {
  g_core    *world_ctx;
  archetype *archetype_ctx;
};

struct system_data {
  g_system start_system; /* A function pointer to a user defined function. */
  type_set requirements; /* Set : [hash(comp name)] */
  int32_t  readonly;
};

#endif