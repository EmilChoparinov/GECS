/*==============================================================================
Associated Files:
    vector.c vector.h
Purpose:
    This is a basic library that uses unrolled arrays to implement a growable
    vector. Contains the basic features of retrieval via index and pushing.
    Deletion is not supported in this structure.
==============================================================================*/

#include "vector.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct table_t table_t;
typedef struct chunk_t chunk_t;

struct chunk_t {
  size_t   len;
  void    *data;
  chunk_t *next;
};

struct table_t {
  void **table;
  size_t len;
  size_t curs;
};

struct vector_t {
  size_t   len;
  size_t   el_size;
  chunk_t *head, *tail;
  table_t  lookup;
};

static chunk_t *new_chunk(vector_t *v);
static int      reserve(vector_t *v);
static int      chunk_destroy(chunk_t *c);

vector_t *vector_make(size_t el_size) {
  vector_t *v;
  chunk_t  *c;

  /* Allocate vector v */
  v = malloc(sizeof(*v));
  if (!v) return NULL;

  /* Initialize vector v */
  v->len = 0;
  v->el_size = el_size;

  /* Initialize and link chunk c to vector v */
  if ((c = new_chunk(v)) == NULL) return NULL;
  v->head = c;
  v->tail = c;

  /* Initialize chunk table. Apply chunk c as first index in table */
  if ((v->lookup.table = (void **)malloc(sizeof(void *))) == NULL) return NULL;
  v->lookup.curs = 1;
  v->lookup.len = 1;
  v->lookup.table[0] = c;

  return v;
}

void vector_free(vector_t *v) {
  chunk_destroy(v->head);
  free(v->lookup.table);
  free(v);
}

int vector_push(vector_t *v, void *el) {
  if (reserve(v) == VECTOR_FAIL) return VECTOR_FAIL;

  /*
    We apply y=b+mx:
        y = loc
        m = el_size
        x = len
        b = data
    We push `el` to location y.
  */
  memcpy(v->tail->data + v->el_size * v->tail->len, el, v->el_size);
  v->len++;
  v->tail->len++;
  return VECTOR_OKAY;
}

void *vector_at(const vector_t *v, size_t loc) {
  size_t   local_pos, chunk_pos;
  chunk_t *c;

  chunk_pos = loc / VECTOR_CHUNK_CAP;
  local_pos = loc % VECTOR_CHUNK_CAP;

  c = v->lookup.table[chunk_pos];
  return c->data + local_pos * v->el_size;
}

void *vector_first(const vector_t *v) { return vector_at(v, 0); }

void *vector_last(const vector_t *v) { return vector_at(v, v->len - 1); }

size_t vector_len(vector_t *v) { return v->len; }

size_t vector_sizeof(void) { return sizeof(vector_t); }

static chunk_t *new_chunk(vector_t *v) {
  chunk_t *c;

  c = malloc(sizeof(*c));
  if (!c) return NULL;

  c->data = malloc(v->el_size * VECTOR_CHUNK_CAP);
  c->len = 0;
  c->next = NULL;

  return c;
}

static int reserve(vector_t *v) {
  chunk_t *c;
  if (v->tail->len == VECTOR_CHUNK_CAP) {
    if ((c = new_chunk(v)) == NULL) return VECTOR_FAIL;

    /* Lookup table reserve procedure (if needed). */
    if (v->lookup.curs == v->lookup.len) {
      v->lookup.len *= 2;
      v->lookup.table =
          realloc(v->lookup.table, v->lookup.len * sizeof(void *));
    }
    v->lookup.table[v->lookup.curs++] = c;
    v->tail->next = c;
    v->tail = c;
  }
  return VECTOR_OKAY;
}

static int chunk_destroy(chunk_t *c) {
  if (!c) return VECTOR_OKAY;
  chunk_destroy(c->next);
  free(c->data);
  free(c);
  return VECTOR_OKAY;
}