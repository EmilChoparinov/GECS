#include "short_vector.h"

struct short_vec_t {
  void *elements;
  void *top;

  int    len;
  size_t capacity;
  size_t el_size;
};

short_vec_t *short_vec_make(size_t el_size, size_t initial_vec_size) {
  short_vec_t *vec;
  if ((vec = malloc(sizeof(*vec))) == NULL) return NULL;

  vec->len = 0;
  vec->capacity = initial_vec_size;
  vec->el_size = el_size;
  if ((vec->elements = malloc(el_size * initial_vec_size)) == NULL) return NULL;
  vec->top = vec->elements;

  return vec;
}

int short_vec_free(short_vec_t *v) {
  free(v->elements);
  free(v);
  return SHORT_VEC_OK;
}

void *short_vec_at(short_vec_t *v, int i) {
  if (i >= v->len) return NULL;
  return v->elements + i * v->el_size;
}

int short_vec_push(short_vec_t *v, void *el) {
  if (v->len == v->capacity)
    if (short_vec_resize(v, v->capacity * 2) == SHORT_VEC_FAIL)
      return SHORT_VEC_FAIL;
  memcpy(v->top, el, v->el_size);
  v->top += v->el_size;
  v->len++;
  return SHORT_VEC_OK;
}

void *short_vec_pop(short_vec_t *v) {
  if (v->len <= 0) return NULL;

  void *el;
  el = v->top - v->el_size;

  v->top -= v->el_size;
  v->len--;

  return el;
}

void *short_vec_first(short_vec_t *v) {
  if (v->len == 0) return NULL;
  return v->elements;
}
void *short_vec_top(short_vec_t *v) {
  if (v->len == 0) return NULL;
  return v->top - v->el_size;
}

int    short_vec_len(short_vec_t *v) { return v->len; }
size_t short_vec_sizeof(void) { return sizeof(short_vec_t); }

int short_vec_resize(short_vec_t *v, size_t resize_to) {
  v->capacity = resize_to;
  v->elements = realloc(v->elements, v->capacity * v->el_size);
  v->top = v->elements + v->len * v->el_size;
  return SHORT_VEC_OK;
}
