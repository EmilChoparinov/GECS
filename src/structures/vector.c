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

/*-------------------------------------------------------
 * Utility Functions
 *-------------------------------------------------------*/
bool __vectors_intersect(any_vec_t *a, any_vec_t *b) {
  if (a->__el_size != b->__el_size) {
    log_warn("[vectors_intersect]: attempted to compare vectors with different "
             "element sizes: %ld, %ld\n",
             a->__el_size, b->__el_size);
    return false;
  }

  for (size_t a_i = 0; a_i < a->length; a_i++) {
    void *a_el = any_vec_at(a, a_i);
    if (any_vec_has(b, a_el)) return true;
  }
  return false;
}