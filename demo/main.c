#include "gecs.h"
#include <pthread.h>
#include <stdio.h>

typedef struct RedColor RedColor;
struct RedColor {
  int x;
};

typedef RedColor     BlueColor; /* We only care about types in this demo */
typedef RedColor     GreenColor;
typedef RedColor     Fall;
typedef struct Vec2d Vec2d;
struct Vec2d {
  int x, y;
};

gid       player, opponent;
g_core_t *world;

void add_ones(g_query_t *q) {
  printf("Entered function!! tid: %lu\n", (uint64_t)pthread_self());
}

int  red_itrs = 0;
void is_red(g_query_t *q) {
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

void subtract_vec2d_once(g_itr *itr, void *arg) {
  Vec2d *coords = gq_field(itr, Vec2d);
  Vec2d  old = *coords;
  coords->x--;
  coords->y--;
  log_error("Subtracting from {x: %d, y: %d} -> {x: %d, y: %d}\n", old.x, old.y,
            coords->x, coords->y);
}

void update_falling_entities(g_query_t *q) {
  g_vec vec = gq_vectorize(q);
  gq_each(vec, subtract_vec2d_once, NULL);
}

int main(void) {
  log_set_level(LOG_ERROR);
  world = g_create_world();

  G_COMPONENT(world, Vec2d);
  G_COMPONENT(world, RedColor);
  G_COMPONENT(world, BlueColor);
  G_COMPONENT(world, GreenColor);
  G_COMPONENT(world, Fall);

  G_SYSTEM(world, add_ones, Vec2d);
  G_SYSTEM(world, is_red, RedColor);
  G_SYSTEM(world, is_blue, BlueColor);
  // G_SYSTEM(world, update_falling_entities, Vec2d, Fall);

  player = g_create_entity(world);
  opponent = g_create_entity(world);

  /* Generate 1000 entities for falling calc tests */
  for (int i = 0; i < 1000; i++) {
    gid entt = g_create_entity(world);
    G_ADD_COMPONENT(world, entt, Vec2d);
    G_ADD_COMPONENT(world, entt, Fall);
    G_SET_COMPONENT(world, entt, Fall, {.x = 0});
    G_SET_COMPONENT(world, entt, Vec2d, {.x = 100, .y = 100});
  }

  printf("player is %ld\n", player);

  G_ADD_COMPONENT(world, player, Vec2d);
  G_ADD_COMPONENT(world, opponent, Vec2d);

  G_ADD_COMPONENT(world, player, RedColor);
  G_ADD_COMPONENT(world, opponent, BlueColor);

  G_SET_COMPONENT(world, opponent, BlueColor, {.x = 55});
  G_SET_COMPONENT(world, player, Vec2d, {.x = 6, .y = 9});
  Vec2d *playerPos = G_GET_COMPONENT(world, player, Vec2d);
  printf("player pos: {x: %d, y: %d}\n", playerPos->x, playerPos->y);

  for (int i = 0; i < 3; i++) {
    g_progress(world);
  }

  g_destroy_world(world);
}
