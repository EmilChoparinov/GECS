/* =========================================================================
    Author: E.D Choparinov, Amsterdam
    Related Files: short_vector.h short_vector.c
    Created On: April 17 2024
    Purpose:
        short_vector is a stand-alone contiguous vector provider. The
        datastructure always guarentees elements in the vector are next
        to each other in memory.
========================================================================= */

#ifndef __HEADER_SHORT_VECTOR_H__
#define __HEADER_SHORT_VECTOR_H__

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define SHORT_VEC_OK 0
#define SHORT_VEC_FAIL -1
#define short_vec_default(el_size) short_vec_make(el_size, 256)

typedef struct short_vec_t short_vec_t;
typedef bool (*compare_cust)(const void *);

short_vec_t *short_vec_make(size_t el_size, size_t initial_vec_size);
int          short_vec_resize(short_vec_t *v, size_t resize_to);
int          short_vec_free(short_vec_t *v);

void *short_vec_at(short_vec_t *v, int i);
int   short_vec_push(short_vec_t *v, void *el);
void *short_vec_pop(short_vec_t *v);
void *short_vec_first(short_vec_t *v);
void *short_vec_top(short_vec_t *v);

void *short_vec_find(short_vec_t *v, compare_cust predicate);
bool  short_vec_has(short_vec_t *v, void *el);

int    short_vec_len(short_vec_t *v);
size_t short_vec_sizeof(void);

#endif