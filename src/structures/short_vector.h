#ifndef __HEADER_SHORT_VECTOR_H__
#define __HEADER_SHORT_VECTOR_H__

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define SHORT_VEC_OK 0
#define SHORT_VEC_FAIL -1
#define short_vec_default(size) short_vec_make(size, 256);

typedef struct short_vec_t short_vec_t;

short_vec_t *short_vec_make(size_t el_size, size_t initial_vec_size);
int          short_vec_resize(short_vec_t *v, size_t resize_to);
int          short_vec_free(short_vec_t *v);

void *short_vec_at(short_vec_t *v, int i);
int   short_vec_push(short_vec_t *v, void *el);
void *short_vec_pop(short_vec_t *v);
void *short_vec_first(short_vec_t *v);
void *short_vec_top(short_vec_t *v);

int    short_vec_len(short_vec_t *v);
size_t short_vec_sizeof(void);

#endif