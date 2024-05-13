/* =========================================================================
  Author: E.D Choparinov, Amsterdam
  Related Files: simple_simulation.h simple_simulation.c
  Created On: May 13 2024
  Purpose:
    This contains a simple simulation where a target and a homing entity
    spawn. The homing entity per frame walks to the target. When hit, the
    target and homing entity will respawn at a random position
========================================================================= */
#include <stdio.h>

#include "gecs.h"

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
      }
    }
  }
}

void system_end_frame(gecs_q_t *q) {}

int main(void) {
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

  gecs_register_system(world, &system_flag_collisions);
  gecs_register_system(world, &system_end_frame);

  homing = gecs_make_entity(world);
  target = gecs_make_entity(world);

  gecs_add_component(world, homing, "vec2", strlen("vec2"));
  vec2_t *vec2_temp = (vec2_t *)gecs_entity_get_by_id(world, homing, "vec2");
  vec2_temp->x = 0;
  vec2_temp->y = 0;

  gecs_add_component(world, homing, "collided", strlen("collided"));
  collided_t *collided_temp =
      (collided_t *)gecs_entity_get_by_id(world, homing, "collided");
  collided_temp->is_colliding = false;

  gecs_add_component(world, homing, "homing_tag", strlen("homing_tag"));

  gecs_add_component(world, target, "vec2", strlen("vec2"));
  vec2_temp = (vec2_t *)gecs_entity_get_by_id(world, target, "vec2");
  vec2_temp->x = 0;
  vec2_temp->y = 0;

  gecs_add_component(world, target, "collided", strlen("collided"));
  collided_temp =
      (collided_t *)gecs_entity_get_by_id(world, target, "collided");
  collided_temp->is_colliding = false;

  gecs_progress(world);

  gecs_complete(world);

  return 0;
}