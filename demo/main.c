/*-------------------------------------------------------
 * This game uses the jump button to jump past the enemy.
 * The enemy spawns on the right side and moves once per
 * second to the left where the player is. The player must
 * jump to dodge the enemy. If the player and enemy
 * intersect, the game ends.
 *-------------------------------------------------------*/
#include "gecs.h"

#include <curses.h>
#include <unistd.h>

#define GAME_WIN_X 15
#define GAME_WIN_Y 5
#define UI_WIN_X   15
#define UI_WIN_Y   2

#define KEY_JUMP ' '

WINDOW *GAME_WIN, *UI_WIN;

bool game_alive = true;

typedef struct Vec2 Vec2;
struct Vec2 {
  int32_t x, y;
};

typedef struct Player Player;
struct Player {
  int8_t  hits_left;      /* Times player can be touched by an enemy */
  int64_t last_jump_tick; /* Only allow jumps to last 1.5 seconds. Set to 0 when
                            not jumping */
};

typedef struct Enemy Enemy;
struct Enemy {
  int64_t last_move_tick; /* Only update enemy when timer hits 0 */
};

gid  PLAYER, ENEMY;
bool hit_this_tick = false;

TAG(gen_enemies);

/* This function will run and update enemies in parallel */
feach(update_enemy, g_pool, entity, {
  Enemy *enemy = gq_field(entity, Enemy);
  Vec2  *pos = gq_field(entity, Vec2);

  /* Only update this enemy once per second */
  log_debug("Processing enemy");
  if (gq_tick_from_pool(entity) - enemy->last_move_tick < 2) return;
  enemy->last_move_tick = gq_tick_from_pool(entity);
  pos->x--;
});

void move_enemies_left_sys(g_query *q) {
  /* Update enemy positions concurrently */
  g_par enemies = gq_vectorize(q);
  log_debug("In enemy system");
  gq_each(enemies, update_enemy, NULL);

  /* Check for entities who have left the screen */
  g_pool seq_enemies = gq_seq(q);
  while (!gq_done(seq_enemies)) {
    GecID *id = gq_field(seq_enemies, GecID);
    if (!gq_id_alive(q, id->id)) return;
    Vec2 *enemy_pos = gq_field(seq_enemies, Vec2);
    if (enemy_pos->x == -1) gq_mark_delete(q, id->id);
    seq_enemies = gq_next(seq_enemies);
  }
}

void player_actions_sys(g_query *q) {
  log_enter;
  if (!gq_id_in(q, PLAYER)) return;
  log_debug("In player system");

  /* Get player sequentially */
  g_pool  player_pool = gq_seq(q);
  Player *player = gq_field(player_pool, Player);
  Vec2   *pos = gq_field(player_pool, Vec2);

  if (gq_tick(q) - player->last_jump_tick < 3) return;

  if (getch() == KEY_JUMP) {
    log_debug("jumped");
    player->last_jump_tick = gq_tick(q);
    pos->y -= 1;
    log_leave;
    return;
  }

  pos->y = GAME_WIN_Y - 2;
  log_leave;
}

void enemy_generator(g_core *w) {
  /* Only generate enemies on every 5 and 7 ticks */
  int64_t tick = w->tick;
  if (!(tick % 35 == 0)) return;
  gid entt = g_create_entity(w);
  G_ADD_COMPONENT(w, entt, Vec2);
  G_ADD_COMPONENT(w, entt, Enemy);
  G_SET_COMPONENT(w, entt, Vec2, {.x = GAME_WIN_X - 2, .y = GAME_WIN_Y - 2});
  G_SET_COMPONENT(w, entt, Enemy, {.last_move_tick = 0});
}

void game_logic(g_core *world) {
  Vec2 *player_pos = G_GET_COMPONENT(world, PLAYER, Vec2);
  log_debug("player: %d,%d", player_pos->y, player_pos->x);

  Player *player_data = G_GET_COMPONENT(world, PLAYER, Player);

  if (hit_this_tick) player_data->hits_left--;
  hit_this_tick = false;

  /* Check if the player has died and if so, Print game over and exit */
  if (player_data->hits_left == 0) game_alive = false;
}

feach(check_collision, g_pool, enemy, {
  Vec2 *player_pos = args;
  Vec2 *enemy_pos = gq_field(enemy, Vec2);
  /* Don't check against yourself in the Vec2 pool */
  if (enemy_pos == player_pos) return;
  if (player_pos->x == enemy_pos->x && player_pos->y == enemy_pos->y)
    hit_this_tick = true;
});
void hit_detection(g_query *q) {
  Vec2 *player_pos = G_GET_COMPONENT(q->world_ctx, PLAYER, Vec2);
  /* Concurrently check all enemy positions against player */
  g_par positions = gq_vectorize(q);
  gq_each(positions, check_collision, player_pos);
}

void render_game(g_core *world) {
  Vec2   *player_pos = G_GET_COMPONENT(world, PLAYER, Vec2);
  Player *player_data = G_GET_COMPONENT(world, PLAYER, Player);

  g_pool enemies = G_GET_POOL(world, Vec2, Enemy);

  /* Scan and put top down, left right */
  wclear(GAME_WIN);
  box(GAME_WIN, 0, 0);
  for (int64_t y = 0; y < GAME_WIN_Y; y++) {
    for (int64_t x = 0; x < GAME_WIN_X; x++) {
      if (y == GAME_WIN_Y - 1 || y == 0) {
        mvwaddch(GAME_WIN, y, x, '=');
        continue;
      }
      if (y == player_pos->y && x == player_pos->x) {
        mvwaddch(GAME_WIN, y, x, 'Y');
        continue;
      }
    }
  }
  while (!gq_done(enemies)) {
    Vec2 *pos = gq_field(enemies, Vec2);
    mvwaddch(GAME_WIN, pos->y, pos->x, '0');
    enemies = gq_next(enemies);
  }

  wclear(UI_WIN);
  mvwprintw(UI_WIN, 0, 0, "Health: %d\n", player_data->hits_left);

  wrefresh(GAME_WIN);
  wrefresh(UI_WIN);
  refresh();
}

int main(void) {
  log_set_level(LOG_ERROR);

  /* Curses Init */
  initscr();
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE); /* make getch non-blocking */
  noecho();

  GAME_WIN = newwin(GAME_WIN_Y, GAME_WIN_X, 0, 0);
  UI_WIN = newwin(UI_WIN_Y, UI_WIN_X, GAME_WIN_Y, 0);

  /* SETUP PHASE */
  g_core *world = g_create_world();

  /* Register Components Step */
  G_COMPONENT(world, Vec2);
  G_COMPONENT(world, Enemy);
  G_COMPONENT(world, Player);

  /* Creating some entities */
  PLAYER = g_create_entity(world);

  G_ADD_COMPONENT(world, PLAYER, Vec2);
  G_ADD_COMPONENT(world, PLAYER, Player);
  G_SET_COMPONENT(world, PLAYER, Vec2, {.x = 2, .y = GAME_WIN_Y - 2});
  G_SET_COMPONENT(world, PLAYER, Player, {.hits_left = 25, .last_jump_tick = 0});

  G_SYSTEM(world, move_enemies_left_sys, Vec2, Enemy);
  G_SYSTEM(world, player_actions_sys, Vec2, Player);
  G_SYSTEM(world, hit_detection, Vec2);

  while (game_alive) {
    enemy_generator(world);
    g_progress(world);
    game_logic(world);
    render_game(world);
    usleep(500000 / 8);
  }

  g_destroy_world(world);
  endwin();
  printf("Game over!\n");

  return 0;
}