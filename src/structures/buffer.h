/* =========================================================================
    Author: E.D Choparinov, Amsterdam
    Related Files: buffer.h buffer.c
    Created On: June 18 2024
    Purpose:
        The purpose of this file is to provide an API that performs various
        low level memory operations on a given memory segment. A pointer to
        the segment is passed in along with the buff_t structure. This
        structure provides a stack-based api that lets you push and pop
        items of varying lengths. All operations provided by this API
        guarentee memory contiguity.

        The structure is fully stack allocated so there is no free for the
        structure, unless you have used the built in allocator provided by
        buff_hinit.

        Note: It is not the job of the buffer operations library to ensure
        the user modifies inside the allocated region. Although, the operations
        provided all contain basic asserts against NULL or invalid values.
========================================================================= */
#ifndef __HEADER_BUFFER_H_
#define __HEADER_BUFFER_H_

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "retcodes.h"

typedef struct buff_t buff_t;
struct buff_t {
  void   *region; /* Pointer to base memory. */
  void   *top;    /* Pointer to the end of base memory operations. */
  int64_t len;    /* Length of objects pushed/pop'd. */
};

/*-------------------------------------------------------
 * Container Operations
 *-------------------------------------------------------*/
buff_t *buff_init(buff_t *b, void *region);
buff_t *buff_hinit(void *region);

void buff_hfree(buff_t *b);

/*-------------------------------------------------------
 * Element Operations
 *-------------------------------------------------------*/
retcode buff_push(buff_t *b, void *item, size_t item_size);
void   *buff_pop(buff_t *b, size_t item_size);
void   *buff_at(buff_t *b, int idx);
void   *buff_skip(buff_t *b, size_t to_skip);

#endif