/*-------------------------------------------------------
 * This game "Rain Dodge" has ASCII characters fall from
 * the top of the screen. The player at the bottom must
 * dodge the rain. They have 3 hit points. The more rain
 * they dodge the higher the score becomes.
 *-------------------------------------------------------*/
#include "gecs.h"
#include <curses.h>
#include <time.h>
#include <unistd.h>

/* Define Window Sizes */
#define GAME_WIN_X 40
#define GAME_WIN_Y 20
#define UI_WIN_X   GAME_WIN_X
#define UI_WIN_Y   6 /* Display hit points, score, and rain stats */

/* Define Simulation Amount */
#define ENTITY_SPAWN_CHUNK 16

/* VIP Entities */
gid     PLAYER, SAMPLER;
g_core *world;

/* timer api */
typedef struct timespec timer;

/* Performance test */
typedef struct TickRateSampler {
  int64_t tick_start;
  int64_t sample_window_ms;
  timer   time_elapsed;
  double  tick_rate;
} TickRateSampler;

typedef struct GameState {
  int8_t  isGameRunning;
  timer   last_render;
  int64_t fps;
  double  wind_dir_y, wind_dir_x;
  double  rain_avg_y, rain_avg_x;
  WINDOW *GAME_WIN;
  WINDOW *UI_WIN;
  WINDOW *GAME_BUFFER;
  WINDOW *UI_BUFFER;
} GameState;
GameState *state; /* Global gamestate ref */

/* Used to track the orientation of the falling rain */
typedef struct Vec2 {
  int32_t x, y;
} Vec2;

/* Used for tracking the position of entities */
typedef struct Position {
  int32_t x, y;
} Position;

typedef struct Timer {
  int64_t wait_in_ms;
  timer   last_update;
  int32_t times_reset;
} Timer;

typedef struct PlayerData {
  int32_t score;
  int32_t hits_left;
  int64_t last_tick_update;
} PlayerData;

TAG(ReadInput);   /* Tag that this entity should read inputs */
TAG(RainSpawner); /* Tag that this entity makes one generator */

/* Static Functions */
static long difftime_ms(timer *end, timer *start) {
  long sec = end->tv_sec - start->tv_sec;
  long nsec = end->tv_nsec - start->tv_nsec;

  // Adjust for cases where nanoseconds difference is negative
  if (nsec < 0) {
    sec -= 1;
    nsec += 1000000000L;
  }

  return (sec * 1000) + (nsec / 1000000);
}

void spawn_rain(Position *pos, Vec2 *v) {
  pos->x = rand() % (GAME_WIN_X - 1) + 1;
  pos->y = 1;
  v->x = rand() % 3 - 2;
  v->y = rand() % 2;
}

void generate_new_rain(g_query *q) {
  g_pool entities = gq_seq(q);
  while (!gq_done(entities)) {
    Timer *on_generate = gq_field(entities, Timer);

    timer now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    long t = difftime_ms(&now, &on_generate->last_update);
    if (t < on_generate->wait_in_ms || on_generate->times_reset > 4) {
      entities = gq_next(entities);
      continue;
    }

    on_generate->times_reset++;
    on_generate->last_update = now;

    for (int i = 0; i < ENTITY_SPAWN_CHUNK; i++) {
      gid new_rain = gq_create_entity(q);
      gq_add(q, new_rain, Position);

      gq_mut(q, new_rain);
      gq_add(q, new_rain, Vec2);
      gq_add(q, new_rain, Timer);

      gq_set(q, new_rain, Timer, {.wait_in_ms = 333, .times_reset = 0});
      Timer *t = gq_get(q, new_rain, Timer);
      clock_gettime(CLOCK_MONOTONIC, &t->last_update);
      spawn_rain(gq_get(q, new_rain, Position), gq_get(q, new_rain, Vec2));
    }
    entities = gq_next(entities);
  }
}

void rain_physics(g_query *q) {
  g_pool entities = gq_seq(q);
  while (!gq_done(entities)) {
    Vec2     *orientation = gq_field(entities, Vec2);
    Position *pos = gq_field(entities, Position);
    Timer    *update_tmr = gq_field(entities, Timer);

    timer now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    long t = difftime_ms(&now, &update_tmr->last_update);
    if (t < update_tmr->wait_in_ms) {
      entities = gq_next(entities);
      continue;
    }
    update_tmr->last_update = now;
    update_tmr->times_reset++;

    /* It takes 3 updates to fall down one more level (to match animations) */
    if (update_tmr->times_reset % 2 == 0) {
      pos->x -= orientation->x;
      pos->y += orientation->y;

      orientation->x = rand() % 4 - 2;
      orientation->y = rand() % 2 + 1;
    }

    if (pos->y >= GAME_WIN_Y || pos->x < 0 || pos->y >= GAME_WIN_X) {
      spawn_rain(pos, orientation);
      PlayerData *data = G_GET_COMPONENT(world, PLAYER, PlayerData);
      data->score++;
    }

    entities = gq_next(entities);
  }
}

void capture_inputs(g_query *q) {
  int       action = getch();
  Position *pos = gq_get(q, PLAYER, Position);
  if (action == 97) pos->x--;
  if (action == 100) pos->x++;

  if (pos->x > GAME_WIN_X) pos->x = 0;
  else if (pos->x < 0) pos->x = GAME_WIN_X - 2;
}

feach(detect_rain, g_pool, rain_entt, {
  void **arglist = args;

  Position *pos = arglist[0];
  bool     *hit = arglist[1];

  if (*hit) return;

  Position *entt_pos = gq_field(rain_entt, Position);
  if (entt_pos->x == pos->x && entt_pos->y == pos->y) {
    *hit = true;
    entt_pos->y = 1;
  }
});
void hit_detection(g_query *q) {
  Position *pos = G_GET_COMPONENT(world, PLAYER, Position);

  g_par rain_entities = gq_vectorize(q);

  bool hit = false;

  void *arglist[3];
  arglist[0] = pos;
  arglist[1] = &hit;

  gq_each(rain_entities, detect_rain, arglist);

  if (hit) {
    PlayerData *data = G_GET_COMPONENT(world, PLAYER, PlayerData);
    data->hits_left--;
  }
}

void game_over_logic(g_query *q) {
  g_pool player = gq_seq(q);

  PlayerData *data = gq_field(player, PlayerData);
  if (data->hits_left == 0) state->isGameRunning = false;
}

void renderer() {
  timer now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  long t = difftime_ms(&now, &state->last_render);
  if (t < (1000 / ((double)state->fps))) return;
  state->last_render = now;

  werase(state->GAME_BUFFER);

  box(state->GAME_BUFFER, 0, 0);
  char rain_assets[] = {
      '`',
      '*',
      ',',
      '\'',
  };

  /* Render game side borders */
  for (int64_t y_coord_draw = 0; y_coord_draw < GAME_WIN_Y; y_coord_draw++) {
    mvwaddch(state->GAME_BUFFER, y_coord_draw, 0, '#');
    mvwaddch(state->GAME_BUFFER, y_coord_draw, GAME_WIN_X - 1, '#');
  }

  /* Render game floor */
  for (int64_t x_coord_draw = 0; x_coord_draw < GAME_WIN_X; x_coord_draw++) {
    mvwaddch(state->GAME_BUFFER, GAME_WIN_Y - 1, x_coord_draw, '=');
  }

  /* Render entities in game world */
  g_pool rain_positions = G_GET_POOL(world, Vec2, Position, Timer);

  while (!gq_done(rain_positions)) {
    Position *pos = gq_field(rain_positions, Position);
    Timer    *t = gq_field(rain_positions, Timer);
    mvwaddch(state->GAME_BUFFER, pos->y, pos->x,
             rain_assets[t->times_reset % 3]);
    rain_positions = gq_next(rain_positions);
  }

  Position   *playerPos = G_GET_COMPONENT(world, PLAYER, Position);
  PlayerData *playerData = G_GET_COMPONENT(world, PLAYER, PlayerData);

  mvwaddch(state->GAME_BUFFER, playerPos->y, playerPos->x, 'Y');

  /* Since we know we are the character, we can load hidden components we
     know exist */
  TickRateSampler *sampler = G_GET_COMPONENT(world, SAMPLER, TickRateSampler);
  mvwprintw(state->UI_WIN, 0, 0, "Score: %d\n", playerData->score);
  mvwprintw(state->UI_WIN, 1, 0, "Hits Left: %d\n", playerData->hits_left);
  mvwprintw(state->UI_WIN, 2, 0, "Wind Dir (X,Y): (%.3f, %.3f)\n",
            state->wind_dir_x, state->wind_dir_y);
  mvwprintw(state->UI_WIN, 3, 0, "Rain Avg Cluster (X,Y): (%.3f, %.3f)\n",
            state->rain_avg_x, state->rain_avg_y);
  mvwprintw(state->UI_WIN, 4, 0, "Recorded tickrate: %.3f / %ld ms \n",
            sampler->tick_rate, sampler->sample_window_ms);

  copywin(state->GAME_BUFFER, state->GAME_WIN, 0, 0, 0, 0, GAME_WIN_Y - 1,
          GAME_WIN_X - 1, 0);
  wrefresh(state->GAME_WIN);
  wrefresh(state->UI_WIN);
  refresh();
}

void calculate_direction(g_query *q) {
  g_pool rain_entt = gq_seq(q);

  int64_t x_dir_sum = 0;
  int64_t y_dir_sum = 0;
  int64_t rain_total = 0;

  while (!gq_done(rain_entt)) {

    Vec2 *dir = gq_field(rain_entt, Vec2);

    x_dir_sum += dir->x;
    y_dir_sum += dir->y;

    rain_total++;
    rain_entt = gq_next(rain_entt);
  }

  state->wind_dir_x = (double)x_dir_sum / rain_total;
  state->wind_dir_y = (double)y_dir_sum / rain_total;
}
void calculate_avg_position(g_query *q) {
  g_pool rain_entt = gq_seq(q);

  int64_t x_pos_sum = 0;
  int64_t y_pos_sum = 0;
  int64_t rain_total = 0;

  while (!gq_done(rain_entt)) {

    Position *dir = gq_field(rain_entt, Position);

    x_pos_sum += dir->x;
    y_pos_sum += dir->y;

    rain_total++;
    rain_entt = gq_next(rain_entt);
  }

  state->rain_avg_x = (double)x_pos_sum / rain_total;
  state->rain_avg_y = (double)y_pos_sum / rain_total;
}

void sample_performance(g_query *q) {
  g_pool sampler = gq_seq(q);

  TickRateSampler *rate_sampler = gq_field(sampler, TickRateSampler);

  timer now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  if (difftime_ms(&now, &rate_sampler->time_elapsed) <=
      rate_sampler->sample_window_ms)
    return;

  int64_t tick_end = q->world_ctx->tick;

  rate_sampler->time_elapsed = now;
  rate_sampler->tick_rate = ((double)tick_end - rate_sampler->tick_start) /
                            rate_sampler->sample_window_ms;
  rate_sampler->tick_start = tick_end;
  FILE *file = fopen("tickrates.txt", "a");
  fprintf(file, ", %.3f", rate_sampler->tick_rate);
  fclose(file);
}

int main(void) {
  srand(time(NULL));
  log_set_level(LOG_ERROR);

  /* Curses Init */
  initscr();
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE); /* Make input reads non-blocking */
  noecho();

  /* SETUP PHASE */
  world = g_create_world();
  world->disable_concurrency =
      0; /* Enable/Disable for benchmark data collection. */

  /* Register components */
  G_COMPONENT(world, GameState);
  G_COMPONENT(world, Vec2);
  G_COMPONENT(world, Position);
  G_COMPONENT(world, PlayerData);
  G_COMPONENT(world, Timer);
  G_COMPONENT(world, TickRateSampler);

  G_TAG(world, ReadInput);
  G_TAG(world, RainSpawner);

  /* Setup game state entity */
  gid gameState = g_create_entity(world);
  G_ADD_COMPONENT(world, gameState, GameState);
  G_SET_COMPONENT(world, gameState, GameState,
                  {.isGameRunning = 1,
                   .GAME_WIN = newwin(GAME_WIN_Y, GAME_WIN_X, 0, 0),
                   .UI_WIN = newwin(UI_WIN_Y, UI_WIN_X, GAME_WIN_Y, 0),
                   .GAME_BUFFER = newwin(GAME_WIN_Y, GAME_WIN_X, 0, 0),
                   .UI_BUFFER = newwin(UI_WIN_Y, UI_WIN_X, GAME_WIN_Y, 0),
                   .fps = 24,
                   .wind_dir_y = 0,
                   .wind_dir_x = 0,
                   .rain_avg_y = 0,
                   .rain_avg_x = 0});
  state = G_GET_COMPONENT(world, gameState, GameState);
  clock_gettime(CLOCK_MONOTONIC, &state->last_render);

  /* Setup player entity */
  PLAYER = g_create_entity(world);
  G_ADD_COMPONENT(world, PLAYER, Position);
  G_ADD_COMPONENT(world, PLAYER, ReadInput);
  G_ADD_COMPONENT(world, PLAYER, PlayerData);
  G_SET_COMPONENT(world, PLAYER, Position,
                  {.x = GAME_WIN_X / 2, .y = GAME_WIN_Y - 2});
  G_SET_COMPONENT(world, PLAYER, PlayerData, {.score = 0, .hits_left = 100});

  gid rainSpawner = g_create_entity(world);
  G_ADD_COMPONENT(world, rainSpawner, RainSpawner);
  G_ADD_COMPONENT(world, rainSpawner, Timer);
  G_SET_COMPONENT(world, rainSpawner, Timer,
                  {.wait_in_ms = 1500, .times_reset = 0});
  Timer *t = G_GET_COMPONENT(world, rainSpawner, Timer);
  clock_gettime(CLOCK_MONOTONIC, &t->last_update);

  /* Setup falling rain entities */
  for (int i = 0; i < ENTITY_SPAWN_CHUNK; i++) {
    gid rain = g_create_entity(world);
    // printf("rain id: %lu\n", rain);
    G_ADD_COMPONENT(world, rain, Position);
    G_ADD_COMPONENT(world, rain, Vec2);
    G_ADD_COMPONENT(world, rain, Timer);
    G_SET_COMPONENT(world, rain, Timer, {.wait_in_ms = 333, .times_reset = 0});
    Timer    *t = G_GET_COMPONENT(world, rain, Timer);
    Position *p = G_GET_COMPONENT(world, rain, Position);
    Vec2     *v = G_GET_COMPONENT(world, rain, Vec2);
    clock_gettime(CLOCK_MONOTONIC, &t->last_update);
    spawn_rain(p, v);
  }

  /* Setup sampler */
  SAMPLER = g_create_entity(world);
  G_ADD_COMPONENT(world, SAMPLER, TickRateSampler);
  TickRateSampler *setup = G_GET_COMPONENT(world, SAMPLER, TickRateSampler);
  clock_gettime(CLOCK_MONOTONIC, &setup->time_elapsed);
  setup->sample_window_ms = 250; /* Sample once per 0.25 seconds */
  setup->tick_rate = 0;
  setup->tick_start = 0;

  /* Player Systems */
  G_SYSTEM(world, capture_inputs, DEFAULT, Position, ReadInput);
  G_SYSTEM(world, game_over_logic, DEFAULT, PlayerData);
  G_SYSTEM(world, hit_detection, DEFAULT, Timer, Vec2, Position);

  /* Simulation Systems */
  G_SYSTEM(world, rain_physics, DEFAULT, Timer, Vec2, Position);
  G_SYSTEM(world, generate_new_rain, DEFAULT, RainSpawner, Timer);
  G_SYSTEM(world, calculate_direction, SYS_READONLY, Timer, Vec2, Position);
  G_SYSTEM(world, calculate_avg_position, SYS_READONLY, Timer, Vec2, Position);

  /* Performance Systems */
  G_SYSTEM(world, sample_performance, DEFAULT, TickRateSampler);

  while (state->isGameRunning) {
    renderer(); /* ncurses hates threading! so i can't use the ECS for this
    // */
    g_progress(world);
    // usleep(1000);
  }

  /* CLEANUP PHASE */
  /* There's a mem corruption bug with system handling somewhere */
  g_destroy_world(world);
  endwin();
  PlayerData *data = G_GET_COMPONENT(world, PLAYER, PlayerData);
  printf("Game over! You dodged %d rain drops\n", data->score);
  return 0;
}