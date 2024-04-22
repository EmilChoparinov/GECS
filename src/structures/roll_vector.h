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