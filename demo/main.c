#include "gecs.h"

typedef struct Vec2 Vec2;
struct Vec2 {
  int32_t x, y;
};

void entity_generator(g_query *q) {}

int main() {
  g_core *world = g_create_world();
  G_COMPONENT(world, Vec2);

  G_SYSTEM(world, entity_generator, Vec2);

  g_create_entity(world);
  g_create_entity(world);
  g_create_entity(world);
  g_create_entity(world);
  g_create_entity(world);
  g_create_entity(world);
//   g_mark_delete(world, g_create_entity(world));

  g_destroy_world(world);
  return 0;
}