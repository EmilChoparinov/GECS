#include "gecs.h"
#include <pthread.h>
#include <stdio.h>

typedef struct RedColor RedColor;
struct RedColor {
  int x;
};

typedef RedColor     BlueColor; /* We only care about types in this demo */
typedef RedColor     GreenColor;
typedef struct Vec2d Vec2d;
struct Vec2d {
  int x, y;
};

gid       player, opponent;
g_core_t *world;

void add_ones(g_query_t *q) {
  log_error("START ADD ONES\n");
  printf("Entered function!! tid: %lu\n", (uint64_t)pthread_self());
}

int  red_itrs = 0;
void is_red(g_query_t *q) {
  log_error("START RED\n");
  printf("Entering red [%d] tid: %lu\n", red_itrs, (uint64_t)pthread_self());

  gid ctx;
  if (gq_id_in(q, player)) {
    ctx = player;
    printf("player selected in red\n");
  } else if (gq_id_in(q, opponent)) {
    ctx = opponent;
    printf("opponent selected in red\n");
  } else {
    log_error("failed to select in red!\n");
    exit(EXIT_FAILURE);
  }

  gq_add(q, ctx, BlueColor);
  gq_set(q, ctx, BlueColor, {.x = 69});
  BlueColor *b = gq_get(q, ctx, BlueColor);
  printf("color blue is: %d\n", b->x);

  gq_rem(q, ctx, RedColor);
  printf("Red finished!!\n");
  red_itrs++;
}

int  blue_itrs = 0;
void is_blue(g_query_t *q) {
  log_error("START BLUE\n");
  printf("Entering blue [%d] tid: %lu \n", blue_itrs, (uint64_t)pthread_self());
  gid ctx;
  if (gq_id_in(q, player)) {
    printf("player selected in blue!!\n");
    ctx = player;
  } else if (gq_id_in(q, opponent)) {
    printf("opponent selected in blue!\n");
    ctx = opponent;
  } else {
    log_error("failed to select in blue!\n");
    exit(EXIT_FAILURE);
  }
  BlueColor *b = gq_get(q, ctx, BlueColor);
  printf("color blue is: %d\n", b->x);
  gq_add(q, ctx, RedColor);
  gq_rem(q, ctx, BlueColor);
  printf("Blue finished!!\n");
  blue_itrs++;
}

int main(void) {
  log_set_level(LOG_WARN);
  world = g_create_world();

  G_COMPONENT(world, Vec2d);
  G_COMPONENT(world, RedColor);
  G_COMPONENT(world, BlueColor);
  G_COMPONENT(world, GreenColor);

  G_SYSTEM(world, add_ones, Vec2d);
  G_SYSTEM(world, is_red, RedColor);
  G_SYSTEM(world, is_blue, BlueColor);

  player = g_create_entity(world);
  opponent = g_create_entity(world);

  printf("player is %ld\n", player);

  G_ADD_COMPONENT(world, player, Vec2d);
  G_ADD_COMPONENT(world, opponent, Vec2d);

  G_ADD_COMPONENT(world, player, RedColor);
  G_ADD_COMPONENT(world, opponent, BlueColor);

  G_SET_COMPONENT(world, opponent, BlueColor, {.x = 55});
  G_SET_COMPONENT(world, player, Vec2d, {.x = 6, .y = 9});
  Vec2d *playerPos = G_GET_COMPONENT(world, player, Vec2d);
  printf("player pos: {x: %d, y: %d}\n", playerPos->x, playerPos->y);

  for (int i = 0; i < 1; i++) {
    g_progress(world);
  }

  g_destroy_world(world);
}
