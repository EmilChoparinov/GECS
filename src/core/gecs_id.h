#ifndef __HEADER_GECS_ID_H__
#define __HEADER_GECS_ID_H__

#include <stdint.h>
#include "gecs_def.h"

/**
 * @brief gec id's are a unique number for all components, entities, and systems
 * 
 */
typedef uint64_t gecs_id_t;

#define gecs_id(T) GECS_UNIQUE##T##ID_

#endif