#include "map.h"

m_bool m_bool_set_to_true(bool b, void *arg) { return true; }

uint64_t hash_bytes(void *ptr, size_t size) {
  /* djb2 by Dan Bernstein
   * http://www.cse.yorku.ca/~oz/hash.html */
  uint64_t       hash = 5381;
  unsigned char *b_ptr = (unsigned char *)ptr;

  for (size_t i = 0; i < size; i++) {
    hash = ((hash << 5) + hash) + b_ptr[i];
  }
  log_info("hash %s (%ld) -> %ld\n", (char *)ptr, size, hash);
  return hash;
}

VECTOR_GEN_C(m_bool);