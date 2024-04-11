#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "gecs.h"
#include "vector.h"

gecs_core_t *gecs_make_world(void) {
  gecs_core_t *core;
  if ((core = malloc(sizeof(*core))) == NULL) return NULL;

  core->tick = 0;
  core->id_generator = 0;
  core->components_registry = vector_make(sizeof(gecs_component_info_t));

  return core;
}

void gecs_progress(gecs_core_t *world) { world->tick++; }

void gecs_complete(gecs_core_t *world) {
  printf("tick: %lu\n", world->tick);
  free(world);
}

void gecs_register_component(gecs_core_t *world, gecs_component_info_t *info) {
  vector_push(world->components_registry, info);
}
