#include <stdio.h>

#include "gecs.h"

typedef struct Vec2d Vec2d;
struct Vec2d {
  int x, y;
};

void add_ones(g_query_t *q) {}

int main(void) {
  g_core_t *world = g_create_world();

  G_COMPONENT(world, Vec2d);

  G_SYSTEM(world, add_ones, Vec2d);

  gid player = g_create_entity(world);
  G_ADD_COMPONENT(world, player, Vec2d);

  Vec2d *playerPos = G_GET_COMPONENT(world, player, Vec2d);
  playerPos->x = 6;
  playerPos->y = 9;

  g_destroy_world(world);
}
