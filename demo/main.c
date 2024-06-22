#include <stdio.h>

#include "gecs.h"

typedef struct RedColor RedColor;
struct RedColor {
  int x;
};

typedef RedColor BlueColor; /* We only care about types in this demo */

typedef struct Vec2d Vec2d;
struct Vec2d {
  int x, y;
};

gid       player;
g_core_t *world;

void add_ones(g_query_t *q) { printf("Entered function!!\n"); }

void is_red(g_query_t *q) {
  printf("Is now red, going to blue!\n");
  gq_add(q, player, BlueColor);
  gq_rem(q, player, RedColor);
}

void is_blue(g_query_t *q) {
  printf("Is now blue, going to red!\n");
  gq_add(q, player, RedColor);
  gq_rem(q, player, BlueColor);
}

int main(void) {
  world = g_create_world();

  G_COMPONENT(world, Vec2d);
  G_COMPONENT(world, RedColor);
  G_COMPONENT(world, BlueColor);

  G_SYSTEM(world, add_ones, Vec2d);
  G_SYSTEM(world, is_red, RedColor);
  G_SYSTEM(world, is_blue, BlueColor);

  player = g_create_entity(world);
  G_ADD_COMPONENT(world, player, Vec2d);
  G_ADD_COMPONENT(world, player, RedColor);

  G_SET_COMPONENT(world, player, Vec2d, {.x = 6, .y = 9});
  Vec2d *playerPos = G_GET_COMPONENT(world, player, Vec2d);
  printf("player pos: {x: %d, y: %d}\n", playerPos->x, playerPos->y);

  for (int i = 0; i < 5; i++) {
    log_debug("---WORLD TICK START---\n");
    g_progress(world);
    log_debug("---WORLD TICK END---\n");
  }

  g_destroy_world(world);
}
