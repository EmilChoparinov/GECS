/* =========================================================================
    Author: E.D Choparinov, Amsterdam
    Related Files: entity.h entity.c
    Created On: July 16 2024
    Purpose:
        The purpose of this file is to house internal entity loading
        routines to condense some of the code that repeats throughout
        the library.
========================================================================= */

#ifndef __HEADER_ENTITY_H__
#define __HEADER_ENTITY_H__

#include "archetype.h"
#include "gecs.h"

archetype *load_entity_archetype(g_core *w, gid entt);

#endif