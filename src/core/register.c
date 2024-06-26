#include "register.h"
#include "logger.h"

/*-------------------------------------------------------
 * Vector Types
 *-------------------------------------------------------*/
VECTOR_GEN_C(bool);
VECTOR_GEN_C(gid);
VECTOR_GEN_C(gint);
VECTOR_GEN_C(fragment);

#include "map.h"

/*-------------------------------------------------------
 * Map Types
 *-------------------------------------------------------*/
MAP_GEN_C(gid, gid);
MAP_GEN_C(gid, gsize);
