#include "str_utils.h"

uint64_t hash_vector(vec *v) {
  return hash_bytes(v->elements, v->length * v->__el_size);
}