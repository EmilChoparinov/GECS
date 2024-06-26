#ifndef __HEADER__GID_H__
#define __HEADER__GID_H__

#include "register.h"

// 64 bits = FFFFFFFF

#define SELECT_ID(id)   (uint32_t)((id & 0xFFFFFFFFFFFFFFFE) >> 1)
#define SELECT_MODE(id) (uint32_t)(id & 0x0000000000000001)

#define STORAGE 0
#define CACHED  1

#define DEL 1
#define ALV 0

#define GID_INCR(id)                                                           \
  id = ((((gid)(SELECT_ID(id)) + 1) << 1) | (SELECT_MODE(id)))

#define GID_SET_MODE(id, mode)                                                 \
  id = ((((gid)(SELECT_ID(id))) << 1) | SELECT_MODE(mode))

// #define SELECT_FLAGS(id) (int32_t)(id & 0x00000000FFFFFFFF)
// #define SELECT_TAGS(id)  (uint32_t)((id & 0x00000000FFFFFFFE) >> 1)
// #define GID_TAG_ICR(id) \
//   id = ((gid)(SELECT_ID(id)) << 32)| (((SELECT_TAGS(id) >> 1) + 1) << 1) | \
//        SELECT_MODE(id);

#endif