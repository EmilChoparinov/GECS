#ifndef __HEADER_GECS_H__
#define __HEADER_GECS_H__

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "gecs_utils.h"

#include "short_vector.h"

#define GECS_OK 0
#define GECS_FAIL -1

typedef int32_t gecs_size_t; // To avoid unsignedness

typedef struct gecs_core_t   gecs_core_t;
typedef struct gecs_entity_t gecs_entity_t;

typedef struct gecs_component_info_t gecs_component_info_t;
struct gecs_component_info_t {
  char *name;
  void *component_data;

  size_t name_len;
  size_t component_size;
};

typedef struct gecs_system_info_t gecs_system_info_t;
struct gecs_system_info_t {
  void (*sys)(gecs_entity_t *entt);
};

/**
 * @brief Create a new ECS instance.
 *
 * @return ecs_core_t* The newly created ECS instance.
 */
gecs_core_t *gecs_make_world(void);

/**
 * @brief Process one tick.
 *
 * @param world The ECS instance to process one tick with.
 */
void gecs_progress(gecs_core_t *world);

/**
 * @brief Free the ECS instance.
 *
 * @param world The ECS instance to free.
 */
void gecs_complete(gecs_core_t *world);

int gecs_register_component(gecs_core_t                  *world,
                            struct gecs_component_info_t *info);

gecs_entity_t *gecs_make_entity(gecs_core_t *world);

int gecs_add_component(gecs_core_t *world, gecs_entity_t *entt,
                       const char *component_name, const size_t len);



#endif