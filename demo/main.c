#include <stdio.h>

#include "gecs.h"

int main(void) {

  gecs_core_t *world = gecs_make_world();

  printf("Hello world!!!");

  gecs_complete(world);

  return 0;
}