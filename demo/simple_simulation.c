/* =========================================================================
  Author: E.D Choparinov, Amsterdam
  Related Files: simple_simulation.h simple_simulation.c
  Created On: May 13 2024
  Purpose:
    This contains a simple simulation where a target and a homing entity
    spawn. The homing entity per frame walks to the target. When hit, the
    target and homing entity will respawn at a random position
========================================================================= */
#include <curses.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>

#include "gecs.h"

#define SCREEN_SIZE_X 15
#define SCREEN_SIZE_Y 10

/*-------------------------------------------------------
 * CONTROL SCHEME
 *-------------------------------------------------------*/
#define TRAVEL_LEFT 'a'
#define TRAVEL_UP 'w'
#define TRAVEL_RIGHT 'd'
#define TRAVEL_DOWN 's'
#define EXIT_GAME 'q'

/*-------------------------------------------------------
 * COMPONENT STRUCTS
 *-------------------------------------------------------*/
typedef struct vec2_t vec2_t;
struct vec2_t {
  int x, y;
};

typedef struct collided_t collided_t;
struct collided_t {
  bool is_colliding;
};

typedef struct homing_tag_t homing_tag_t;
struct homing_tag_t {
  char _;
};

typedef struct char_tag_t char_tag_t;
struct char_tag_t {
  char _;
};

/*-------------------------------------------------------
 * CURSES HANDLERS
 *-------------------------------------------------------*/
static void on_curses_finish(int sig);

/*-------------------------------------------------------
 * GLOBAL VARS
 *-------------------------------------------------------*/
WINDOW *game, *logs;

/*-------------------------------------------------------
 * SYSTEMS
 *-------------------------------------------------------*/
void system_flag_collisions(gecs_q_t *q) {
  gecs_q_define(q, (char *[]){"vec2", "collided", NULL});
  short_vec_t *entts = gecs_q_select_sub_entities(q);

  int            cnt = short_vec_len(entts);
  gecs_entity_t *entt1, *entt2;

  for (int entt1_idx = 0; entt1_idx < cnt; entt1_idx++) {
    entt1 = (gecs_entity_t *)short_vec_at(entts, entt1_idx);

    for (int entt2_idx = 0; entt2_idx < cnt; entt2_idx++) {
      entt2 = (gecs_entity_t *)short_vec_at(entts, entt2_idx);
      if (entt1 == entt2) continue;

      vec2_t *entt1_pos = (vec2_t *)gecs_entity_get(entt1, "vec2");
      vec2_t *entt2_pos = (vec2_t *)gecs_entity_get(entt2, "vec2");

      if (entt1_pos->x == entt2_pos->x && entt1_pos->y == entt2_pos->y) {
        collided_t *entt1_coll =
            (collided_t *)gecs_entity_get(entt1, "collided");
        collided_t *entt2_coll =
            (collided_t *)gecs_entity_get(entt2, "collided");
        entt1_coll->is_colliding = true;
        entt2_coll->is_colliding = true;
        wprintw(logs, "colliding\n");
      }
    }
  }
  short_vec_free(entts);
}

void system_move_char(gecs_q_t *q) {
  gecs_q_define(q, (char *[]){"vec2", "char_tag", NULL});
  short_vec_t *entts =
      gecs_q_select_from_components(q, (char *[]){"vec2", "char_tag", NULL});
  gecs_entity_t *player = short_vec_at(entts, 0);
  vec2_t        *pos = gecs_entity_get(player, "vec2");

  wprintw(logs, "player vec pointer: %p\n", pos);

  char x = getch();

  switch (x) {
  case TRAVEL_DOWN:
    pos->y = (pos->y + 1) % SCREEN_SIZE_Y;
    break;
  case TRAVEL_UP:
    pos->y--;
    if (pos->y == -1) pos->y = SCREEN_SIZE_Y - 1;
    break;
  case TRAVEL_LEFT:
    pos->x--;
    if (pos->x == -1) pos->x = SCREEN_SIZE_X - 1;
    break;
  case TRAVEL_RIGHT:
    pos->x = (pos->x + 1) % SCREEN_SIZE_X;
    break;
  case EXIT_GAME:
    on_curses_finish(0);
  default:
    break;
  }
}

void system_render(gecs_q_t *q) {
  gecs_q_define(q, (char *[]){"vec2", NULL});
  short_vec_t *entts = gecs_q_select_sub_entities(q);

  int            y, x; /* coords */
  int            entt_i;
  gecs_entity_t *entt;
  vec2_t        *pos;           /* Position of entt object */
  bool           print_as_entt; /* Print O instead of . */
  for (y = 0; y < SCREEN_SIZE_Y; y++) {
    for (x = 0; x < SCREEN_SIZE_X; x++) {

      print_as_entt = false;
      for (entt_i = 0; entt_i < short_vec_len(entts); entt_i++) {
        entt = (gecs_entity_t *)short_vec_at(entts, entt_i);

        pos = (vec2_t *)gecs_entity_get(entt, "vec2");
        if (pos->x == x && pos->y == y) {
          print_as_entt = true;
          break;
        }
      }
      if (print_as_entt)
        mvwaddch(game, y, x, '0');
      else
        mvwaddch(game, y, x, '.');
    }
  }

  short_vec_free(entts);

  wrefresh(logs);
  wrefresh(game);
  refresh();
}
void system_end_frame(gecs_q_t *q) {
  gecs_q_define(q, (char *[]){"collided", NULL});
  short_vec_t *entts, *temp_entts;

  entts = gecs_q_select_sub_entities(q);
  gecs_entity_t *entt, *homing;
  collided_t    *is_col;

  bool collision_occured = true;

  /* are both entities colliding? */
  for (int i = 0; i < short_vec_len(entts); i++) {
    entt = short_vec_at(entts, i);
    is_col = gecs_entity_get(entt, "collided");
    if (!is_col->is_colliding) {
      collision_occured = false;
      break;
    }
  }

  /* Move the dot before the next frame calls */
  if (collision_occured) {
    temp_entts = gecs_q_select_from_components(
        q, (char *[]){"homing_tag", "vec2", NULL});

    homing = short_vec_at(temp_entts, 0);

    vec2_t *homing_vec = gecs_entity_get(homing, "vec2");
    wprintw(logs, "homing vec pointer: %p\n", homing_vec);
    homing_vec->x = rand() % SCREEN_SIZE_X;
    homing_vec->y = rand() % SCREEN_SIZE_Y;

    short_vec_free(temp_entts);
  }

  /* Reset collision components */
  for (int i = 0; i < short_vec_len(entts); i++) {
    is_col = gecs_entity_get(short_vec_at(entts, i), "collided");
    is_col->is_colliding = false;
  }

  short_vec_free(entts);
}

int main(void) {
  /* Misc initialization */
  srand(0);

  /* Curses initialization */
  int   parent_x, parent_y;
  float ratio = 0.5f;

  (void)signal(SIGINT, on_curses_finish);
  (void)initscr();
  keypad(stdscr, TRUE);
  // (void)curs_set(FALSE);
  (void)noecho();

  getmaxyx(stdscr, parent_y, parent_x);

  game = newwin(parent_y * ratio, parent_x, 0, 0);
  logs = newwin(parent_y * ratio, parent_x, parent_y - (parent_y * ratio), 0);

  gecs_size_t homing, target;

  gecs_core_t *world = gecs_make_world();

  gecs_register_component(
      world, &(gecs_component_info_t){.component_data = NULL,
                                      .component_size = sizeof(vec2_t),
                                      .name = "vec2",
                                      .name_len = strlen("vec2")});
  gecs_register_component(
      world, &(gecs_component_info_t){.component_data = NULL,
                                      .component_size = sizeof(collided_t),
                                      .name = "collided",
                                      .name_len = strlen("collided")});
  gecs_register_component(
      world, &(gecs_component_info_t){.component_data = NULL,
                                      .component_size = sizeof(homing_tag_t),
                                      .name = "homing_tag",
                                      .name_len = strlen("homing_tag")});
  gecs_register_component(
      world, &(gecs_component_info_t){.component_data = NULL,
                                      .component_size = sizeof(char_tag_t),
                                      .name = "char_tag",
                                      .name_len = strlen("char_tag")});

  gecs_register_system(world, &system_render);
  gecs_register_system(world, &system_move_char);
  gecs_register_system(world, &system_flag_collisions);
  gecs_register_system(world, &system_end_frame);

  homing = gecs_make_entity(world);
  target = gecs_make_entity(world);

  gecs_add_component(world, homing, "vec2", strlen("vec2"));
  vec2_t *vec2_temp = (vec2_t *)gecs_entity_get_by_id(world, homing, "vec2");
  vec2_temp->x = rand() % SCREEN_SIZE_X;
  vec2_temp->y = rand() % SCREEN_SIZE_Y;

  gecs_add_component(world, homing, "collided", strlen("collided"));
  collided_t *collided_temp =
      (collided_t *)gecs_entity_get_by_id(world, homing, "collided");
  collided_temp->is_colliding = false;

  gecs_add_component(world, homing, "homing_tag", strlen("homing_tag"));

  gecs_add_component(world, target, "vec2", strlen("vec2"));
  vec2_temp = (vec2_t *)gecs_entity_get_by_id(world, target, "vec2");
  vec2_temp->x = rand() % SCREEN_SIZE_X;
  vec2_temp->y = rand() % SCREEN_SIZE_Y;

  gecs_add_component(world, target, "collided", strlen("collided"));
  collided_temp =
      (collided_t *)gecs_entity_get_by_id(world, target, "collided");
  collided_temp->is_colliding = false;

  gecs_add_component(world, target, "char_tag", strlen("char_tag"));

  while (true) gecs_progress(world);

  gecs_complete(world);

  on_curses_finish(0);

  return 0;
}

static void on_curses_finish(int sig) {
  endwin();
  exit(0);
}