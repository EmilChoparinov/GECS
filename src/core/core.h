#ifndef __HEADER_CORE_H__
#define __HEADER_CORE_H__

#include "gecs.h"

#define AS_REAL       0
#define AS_SIMULATION 1

g_core *g_create_world_internal(int32_t flags);

#endif