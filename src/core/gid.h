/* =========================================================================
    Author: E.D Choparinov, Amsterdam
    Related Files: gid.h gid.c
    Created On: July 09 2024
    Purpose:
        The purpose of this file is to implement the smart entity IDs and
        runtime tagging. Tags are not currently implemented!
========================================================================= */

#ifndef __HEADER__GID_H__
#define __HEADER__GID_H__

#include "types.h"
#include <stdatomic.h>

#define SELECT_ID(id)   (uint32_t)((id & 0xFFFFFFFFFFFFFFFE) >> 1)
#define SELECT_MODE(id) (uint32_t)(id & 0x0000000000000001)

#define STORAGE 0
#define CACHED  1

#define DEL 1
#define ALV 0

/* GID_INCR is not thread safe. */
#define GID_INCR(id)                                                           \
  id = ((((gid)(SELECT_ID(id)) + 1) << 1) | (SELECT_MODE(id)))

/* GID_SET_MODE is not thread safe. */
#define GID_SET_MODE(id, mode)                                                 \
  id = ((((gid)(SELECT_ID(id))) << 1) | SELECT_MODE(mode))

gid gid_atomic_incr(atomic_uint_least64_t *idgen);
void gid_atomic_set(atomic_uint_least64_t *idgen, uint64_t MODE);

#endif