/* =========================================================================
  Author: E.D Choparinov, Amsterdam
  Related Files: gecs.h gecs.c
  Created On: April 23 2024
  Purpose:
    This is the main file in the gecs framework. Include only this file
    to access the GECS framework.
========================================================================= */

#ifndef __HEADER_GECS_H__
#define __HEADER_GECS_H__

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "gecs_utils.h"
#include "short_vector.h"

/*-------------------------------------------------------
 * TYPE DEFINITIONS AND HIDDEN STRUCTS
 *-------------------------------------------------------
 * Contains global type definitions used internally and should also be used
 * externally in user code.
 *
 * RETURN CONSTANT MACROS:
 *  GECS_OK, GECS_FAIL
 *
 * TYPES:
 *  gecs_size_t
 *
 * STRUCTS OF INTEREST:
 *  gecs_core_t, gecs_entity_t */

#define GECS_OK 0
#define GECS_FAIL -1

typedef int32_t gecs_size_t; /* To avoid unsignedness. */

/* Core struct. This hidden struct is the main object used to internally track
 * ECS progression. */
typedef struct gecs_core_t gecs_core_t;

/* This hidden struct contains data for tracking a single entity inside a
 *  gecs_core_t struct. */
typedef struct gecs_entity_t gecs_entity_t;

/*-------------------------------------------------------
 * FRAMEWORK METADATA COLLECTION STRUCTS
 *-------------------------------------------------------
 * All of the following structs are used to collect information from the
 * framework user to generate associated ECS internal structs.
 *
 * METADATA TYPE MAP:
 *  gecs_component_info_t -> ECS component
 *  gecs_system_info_t    -> ECS system */

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
  char *component_names[]; /* use NULL sentinel at the end. */
};

/*-------------------------------------------------------
 * FRAMEWORK CREATION FUNCTIONS
 *-------------------------------------------------------
 * Collection of functions used to create, destroy, and modify the gecs_core_t
 * struct, the world struct.
 *
 * FUNCTIONS:
 *  gecs_make_world, gecs_progress, gecs_complete
 * */

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

/*-------------------------------------------------------
 * ECS GECS FUNCTIONS
 *-------------------------------------------------------
 * Collection of functions used to add to gecs_core_t standard ECS objects like
 * components, systems, or entities.
 *
 * REGISTER FUNCTIONS:
 *  gecs_register_component, gecs_register_system
 *
 * CREATION FUNCTIONS:
 *  gecs_make_entity
 *
 * MODIFY FUNCTIONS:
 *  gecs_add_component
 * */

/**
 * @brief Add to `world` a new component that contains `info`
 *
 * @param world The world to add to
 * @param info The metadata related to the component to be added
 * @return GECS_OK, GECS_FAIL
 */
int gecs_register_component(gecs_core_t *world, gecs_component_info_t *info);

/**
 * @brief Add to `world` a new system that contains `info`
 *
 * @param world The world to add to
 * @param info The metadata related to the system to be added
 * @return GECS_OK, GECS_FAIL
 */
int gecs_register_system(gecs_core_t *world, gecs_system_info_t *info);

/**
 * @brief Creates a new and empty entity
 *
 * @param world The world the entity will spawn from
 * @return gecs_entity_t* Direct reference pointer to the entity
 */
gecs_entity_t *gecs_make_entity(gecs_core_t *world);

/**
 * @brief Add a new component to world
 *
 * @param world The world to add to
 * @param entt The entity to add the component to
 * @param component_name The component name to add to the entity
 * @param len The length of the component name
 * @return GECS_OK, GECS_FAIL
 */
int gecs_add_component(gecs_core_t *world, gecs_entity_t *entt,
                       const char *component_name, const size_t len);

#endif