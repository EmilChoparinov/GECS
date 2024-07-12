#include "gid.h"

/* These two atomic functions use the CAS weak operation because each function
   performs a single operation on the atomic integer. Most likely using this
   will cause speedups even though weak can spuriously fail. */

gid gid_atomic_incr(atomic_uint_least64_t *atomic_idgen) {
  gid idgen, expected;
  while (1) {
    idgen = atomic_load(atomic_idgen);
    expected = idgen;

    GID_INCR(idgen);
    if (atomic_compare_exchange_weak(atomic_idgen, &expected, idgen))
      return idgen;
  }
}

void gid_atomic_set(atomic_uint_least64_t *atomic_idgen, uint64_t MODE) {
  gid idgen, expected;
  while (1) {
    idgen = atomic_load(atomic_idgen);
    expected = idgen;

    GID_SET_MODE(idgen, MODE);
    if (atomic_compare_exchange_weak(atomic_idgen, &expected, idgen)) return;
  }
}