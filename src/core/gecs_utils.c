#include "gecs_utils.h"

const unsigned long hash(const char *str) {
  /* djb2 by Dan Bernstein
   * http://www.cse.yorku.ca/~oz/hash.html */
  unsigned long hash = 5381;
  int           c;

  while ((c = *str++)) hash = ((hash << 5) + hash) + c;
  return hash;
}
