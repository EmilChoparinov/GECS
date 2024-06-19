#include "vector.h"

VECTOR_GEN_C(any);

any_vec_t *vec_unknown_type_init(any_vec_t *v, size_t el_size) {
  any_vec_construct(v);
  v->__el_size = el_size;
  v->element_head = calloc(v->__size, v->__el_size);
  assert(v->element_head);
  return v;
}

any_vec_t *vec_unknown_type_heap_init(size_t el_size) {
  any_vec_t *v = malloc(sizeof(*v));
  assert(v);
  vec_unknown_type_init(v, el_size);
  return v;
}
