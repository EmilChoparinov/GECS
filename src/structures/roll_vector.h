/* =========================================================================
    Author: E.D Choparinov, Amsterdam
    Related Files: roll_vector.h roll_vector.c
    Created On: April 22 2024
    Purpose:
        roll_vector is an extension of the short_vector data structure in
        which pointer based chunking is supported. See
        https://en.wikipedia.org/wiki/Unrolled_linked_list which this
        structure is based on.

        This vector has two modes: stack or be a generic array.
========================================================================= */

#ifndef __HEADER_ROLL_VECTOR_H__
#define __HEADER_ROLL_VECTOR_H__

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "short_vector.h"

#define ROLL_VEC_OK 0
#define ROLL_VEC_FAIL -1
#define roll_vec_default(size) roll_vec_make(size, 256);

typedef struct roll_vec_t roll_vec_t;

roll_vec_t *roll_vec_make(size_t el_size, size_t chunk_size, int chunk_count);
int         roll_vec_free(roll_vec_t *v);

void *roll_vec_at(roll_vec_t *v, int i);
int   roll_vec_push(roll_vec_t *v, void *el);
void *roll_vec_pop(roll_vec_t *v);
void *roll_vec_first(roll_vec_t *v);
void *roll_vec_top(roll_vec_t *v);

int    roll_vec_len(roll_vec_t *v);
size_t roll_vec_sizeof(void);

#endif