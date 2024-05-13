#ifndef __VECTOR_H__
#define __VECTOR_H__

#define VECTOR_FAIL -1
#define VECTOR_OKAY 0
#define VECTOR_CHUNK_CAP 2048

#include <stdlib.h>


typedef struct vector_t vector_t;

vector_t *vector_make(size_t el_size);
void      vector_free(vector_t *v);

int   vector_push(vector_t *v, void *el);
void *vector_at(const vector_t *v, size_t loc);
void *vector_first(const vector_t *v);
void *vector_last(const vector_t *v);

size_t vector_len(vector_t *v);
size_t vector_sizeof(void);

#endif
