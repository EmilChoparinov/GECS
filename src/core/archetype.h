/* =========================================================================
    Author: E.D Choparinov, Amsterdam
    Related Files: archetype.h archetype.c
    Created On: July 09 2024
    Purpose:
        The purpose of this file is to house the implementation details of
        archetypes from the paper.
========================================================================= */
#ifndef __HEADER_ARCHETYPES_H__
#define __HEADER_ARCHETYPES_H__

#include "gecs.h"
#include "gid.h"
#include "logger.h"
#include "str_utils.h"

/* We define the empty archetype to be an arbitrary unique address containing
   id = 0; */
extern archetype empty_archetype;

/* Initializes a new archetype 'a' with the context world 'w'
   and types 'types'. */
void init_archetype(g_core *w, archetype *a, hash_vec *types);

/* Free's archetype a */
void free_archetype(archetype *a);

/* Given a string of types "ComponentA,ComponentB,ComponentC", hash each item
   delimited by ',' and sort the vector so that it is ordered. */
void archetype_key(char *types, hash_vec *key);

/* Transition an entity from its current position to a new archetype with
   'types' */
void delta_transition(g_core *w, gid entt, hash_vec *to_key);

/* Creates a new thread and spins up this archetype. Use only once */
void subthread_archetype(archetype *a);

/* Performs the thread process on the current thread instead of being managed
   on a different one */
void archetype_perform_process(g_core *w, archetype *process_arch);

#endif