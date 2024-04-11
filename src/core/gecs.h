#ifndef __HEADER_GECS_H__
#define __HEADER_GECS_H__

#include <assert.h>

#include "vector.h"

/* Import all headers from other core parts of the library */
#include "gecs_def.h"
#include "gecs_id.h"

typedef struct gecs_core_t           gecs_core_t;
typedef struct gecs_component_info_t gecs_component_info_t;

struct gecs_core_t {
  uint64_t  tick;
  gecs_id_t id_generator;

  vector_t *components_registry;
};

struct gecs_component_info_t {
  const gecs_size_t size;
  const char       *name;
  gecs_id_t         id;
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

/**
 * @brief Adds the component info to the world object. No instances are done
 * this is just simply a registration step with the framework.
 *
 * @param world The ECS instance
 * @param info The component info to generate with later
 */
void gecs_register_component(gecs_core_t *world, gecs_component_info_t *info);

#define GECS_WORLD_GEN_ID(world, T)                                            \
  gecs_id_t gecs_id(T) = ++world->id_generator;

#define GECS_REG_COMPONENT(world, component)                                   \
  GECS_WORLD_GEN_ID(world, component);                                         \
  {                                                                            \
    gecs_component_info_t cinfo = {.size = sizeof(#component),                 \
                                   .name = #component,                         \
                                   .id = gecs_id(component)};                  \
    gecs_register_component(world, &cinfo);                                    \
  }                                                                            \
  assert(gecs_id(component) != 0);                                             \

#endif