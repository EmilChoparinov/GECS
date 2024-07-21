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
  int8_t score;           /* Score tracking */
};

typedef struct Enemy Enemy;
struct Enemy {
  int64_t last_move_tick; /* Only update enemy when timer hits 0 */
};

gid PLAYER, ENEMY;

TAG(is_player);
TAG(is_enemy);

void spawn_enemy(g_core *world) {
  ENEMY = g_create_entity(world);
  G_ADD_COMPONENT(world, ENEMY, Vec2);
  G_ADD_COMPONENT(world, ENEMY, Enemy);
  G_SET_COMPONENT(world, ENEMY, Vec2,
                  {.x = GAME_WIN_X - 2, .y = GAME_WIN_Y - 2});
  G_SET_COMPONENT(world, ENEMY, Enemy, {.last_move_tick = 0});
}

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

void game_logic(g_core *world) {
  Vec2 *player_pos = G_GET_COMPONENT(world, PLAYER, Vec2);
  Vec2 *enemy_pos = G_GET_COMPONENT(world, ENEMY, Vec2);
  log_debug("enemy: %d,%d", enemy_pos->y, enemy_pos->x);
  log_debug("player: %d,%d", player_pos->y, player_pos->x);

  Player *player_data = G_GET_COMPONENT(world, PLAYER, Player);

  if (player_pos->x == enemy_pos->x) {
    /* If the y's are the same, we take a hit. If the y's are different, we
       take add to your score. */
    if (player_pos->y == enemy_pos->y) {
      player_data->hits_left--;
      g_mark_delete(world, ENEMY);
      spawn_enemy(world);
    } else {
      /* Process score only on even ticks because im lazy */
      if (world->tick % 2 == 0) player_data->score++;
    }
  }

  /* Check if the player has died and if so, Print game over and exit */
  if (player_data->hits_left == 0) {
    game_alive = false;
  }

  /* Check if enemy made it to the left side. If so, delete and respawn */
  if (enemy_pos->x == -1) {
    g_mark_delete(world, ENEMY);
    spawn_enemy(world);
  }
}

void render_game(g_core *world) {
  Vec2 *player_pos = G_GET_COMPONENT(world, PLAYER, Vec2);
  Vec2 *enemy_pos = G_GET_COMPONENT(world, ENEMY, Vec2);

  Player *player_data = G_GET_COMPONENT(world, PLAYER, Player);

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
      if (y == enemy_pos->y && x == enemy_pos->x) {
        mvwaddch(GAME_WIN, y, x, '0');
        continue;
      }
      mvwaddch(GAME_WIN, y, x, '.');
    }
  }

  wclear(UI_WIN);
  mvwprintw(UI_WIN, 0, 0, "Score: %d\n", player_data->score);
  mvwprintw(UI_WIN, 1, 0, "Hits Left: %d\n", player_data->hits_left);

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
  G_SET_COMPONENT(world, PLAYER, Player, {.hits_left = 3, .last_jump_tick = 0});

  spawn_enemy(world);
  G_SYSTEM(world, move_enemies_left_sys, Vec2, Enemy);
  G_SYSTEM(world, player_actions_sys, Vec2, Player);

  while (game_alive) {
    g_progress(world);
    game_logic(world);
    render_game(world);
    usleep(500000 / 4);
  }

  g_destroy_world(world);
  endwin();
  printf("Game over!\n");

  return 0;
}