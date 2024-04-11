#ifndef __HEADER_SV_H__
#define __HEADER_SV_H__

#define SV_START_SIZE 64

#include <stdlib.h>

typedef struct small_vector_t small_vector_t;

small_vector_t *sv_make(size_t el_size);

void sv_destroy(small_vector_t *s);

void  sv_push(small_vector_t *s, void *el);
void *sv_pop(small_vector_t *s);
void *sv_at(small_vector_t *s, size_t loc);

#endif