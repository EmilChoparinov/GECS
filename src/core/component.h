/* =========================================================================
    Author: E.D Choparinov, Amsterdam
    Related Files: component.h component.c
    Created On: July 13 2024
    Purpose:
        The purpose of this file is to expose an internal interface for
        interacting with components inside the engine. The reasoning
        behind why to use these functions instead of the component
        functions defined in `gecs.h` is because those use string inputs.
        These use the `types.h` data structures.
========================================================================= */

#ifndef __HEADER_COMPONENT_H__
#define __HEADER_COMPONENT_H__

#include "gecs.h"

void  _g_add_component(g_core *w, gid entt, hash_vec *new_types);
void *_g_get_component(g_core *w, gid entt, gid type);
void  _g_set_component(g_core *w, gid entt, gid type, void *comp_data);
#endif