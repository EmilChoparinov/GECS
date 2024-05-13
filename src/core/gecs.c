#include "gecs.h"

/*-------------------------------------------------------
 * STRUCTS AND TYPE DEFINITIONS
 *-------------------------------------------------------*/

struct gecs_entity_t {
  short_vec_t *components; /* of pointers to declared components */
  bool         is_alive;
  gecs_size_t  id;
};

struct gecs_core_t {
  gecs_size_t  id_gen;
  short_vec_t *systems;            /* of gecs_system_t */
  short_vec_t *entities;           /* of gecs_entity_t */
  short_vec_t *component_registry; /* of component_t */
};

typedef struct gecs_component_t gecs_component_t;
struct gecs_component_t {
  gecs_size_t name_hash;
  size_t      component_size;
  void       *component;
};

typedef struct gecs_system_t gecs_system_t;
struct gecs_system_t {
  void (*run_sys)(gecs_entity_t *entt);
  short_vec_t *components_to_query; /* of gecs_size_t id's */
};

/*-------------------------------------------------------
 * STATIC FUNCTION DEFINITIONS
 *-------------------------------------------------------*/

static const gecs_size_t name_to_id(const char *str, const size_t len);
static const bool world_has_component(const gecs_core_t *world, char *name,
                                      size_t len);

/*-------------------------------------------------------
 * MAIN WORLD FUNCTIONS
 *-------------------------------------------------------*/

gecs_core_t *gecs_make_world(void) {
  gecs_core_t *world;
  if ((world = malloc(sizeof(*world))) == NULL) return NULL;

  if ((world->systems = short_vec_make(sizeof(gecs_system_t), 1)) == NULL)
    return NULL;
  if ((world->entities = short_vec_make(sizeof(gecs_entity_t), 1)) == NULL)
    return NULL;
  if ((world->component_registry =
           short_vec_make(sizeof(gecs_component_t), 1)) == NULL)
    return NULL;

  world->id_gen = 0;

  return world;
}

void gecs_progress(gecs_core_t *world) {
  /** Progression Algorithm:
   *    1. Linear iteration through all systems in REGISTRATION ORDER
   *    2. Linear iteration through all entities in REGISTRATION ORDER
   *    3. For each entity, check if the entities components are a subset of
   *       the components list for the system.
   *    5. If yes, run the entity through the system
   *    6. If no, skip the system and move on to next entity
   */

  int itr_sys, itr_entt, itr_sys_comp, itr_comp, sys_cnt, entt_cnt, comp_cnt,
      sys_comp_cnt;

  bool is_subset;

  gecs_size_t       id_sub, *id_super;
  gecs_system_t    *sys;
  gecs_entity_t    *entt;
  gecs_component_t *comp;

  sys_cnt = short_vec_len(world->systems);
  entt_cnt = short_vec_len(world->entities);
  for (itr_sys = 0; itr_sys < sys_cnt; itr_sys++) {
    sys = (gecs_system_t *)short_vec_at(world->systems, itr_sys);
    for (itr_entt = 0; itr_entt < entt_cnt; itr_entt++) {
      entt = (gecs_entity_t *)short_vec_at(world->entities, itr_entt);

      /* Subset check. */
      is_subset = false;
      comp_cnt = short_vec_len(entt->components);
      for (itr_comp = 0; itr_comp < comp_cnt; itr_comp++) {
        comp = (gecs_component_t *)short_vec_at(entt->components, itr_comp);
        id_sub = comp->name_hash;

        /* Find match. */
        sys_comp_cnt = short_vec_len(sys->components_to_query);
        for (itr_sys_comp = 0; itr_sys_comp < sys_comp_cnt; itr_sys_comp++) {
          id_super = (gecs_size_t *)short_vec_at(sys->components_to_query,
                                                 itr_sys_comp);

          /* Found. */
          if (id_sub == *id_super) {
            sys->run_sys(entt);
            is_subset = true;
            break;
          }
        }

        if (!is_subset) break;
      }
    }
  }
}

void gecs_complete(gecs_core_t *world) {
  int            itr;
  gecs_system_t *sys;
  gecs_entity_t *entt;

  // Free the components list of each entity
  for (itr = 0; itr < short_vec_len(world->entities); itr++) {
    entt = (gecs_entity_t *)short_vec_at(world->entities, itr);
    short_vec_free(entt->components);
  }
  // Free the entity list
  short_vec_free(world->entities);

  // Free the system info of each system
  for (itr = 0; itr < short_vec_len(world->systems); itr++) {
    sys = (gecs_system_t *)short_vec_at(world->systems, itr);
    short_vec_free(sys->components_to_query);
  }
  short_vec_free(world->systems);
}

/*-------------------------------------------------------
 * REGISTRATION FUNCTIONS
 *-------------------------------------------------------*/

int gecs_register_component(gecs_core_t *world, gecs_component_info_t *info) {
  gecs_size_t name_to_add;

  name_to_add = name_to_id(info->name, info->name_len);

  // Check to make sure there exists no other component with the same name/id.
  if (world_has_component(world, info->name, info->name_len)) return GECS_FAIL;

  short_vec_push(world->component_registry,
                 &(gecs_component_t){.component_size = info->component_size,
                                     .name_hash = name_to_add,
                                     .component = NULL});
  return GECS_OK;
}

int gecs_register_system(gecs_core_t *world, gecs_system_info_t *info) {

  int         itr;
  gecs_size_t component_id;
  char       *component_name;

  short_vec_t  *comp_ids;
  gecs_system_t gec_sys;

  /* Check to make sure each component has already been registered. */
  comp_ids = short_vec_default(sizeof(gecs_size_t));
  for (itr = 0; info->component_names[itr] != NULL; itr++) {
    component_name = info->component_names[itr];
    component_id = name_to_id(component_name, strlen(component_name));

    if (!world_has_component(world, component_name, strlen(component_name)))
      return GECS_FAIL;

    short_vec_push(comp_ids, &component_id);
  }

  gec_sys.components_to_query = comp_ids;
  gec_sys.run_sys = info->sys;

  short_vec_push(world->systems, &gec_sys);

  return GECS_OK;
}

/*-------------------------------------------------------
 * ENTITY MANIPULATION FUNCTIONS
 *-------------------------------------------------------*/

gecs_entity_t *gecs_make_entity(gecs_core_t *world) {
  gecs_entity_t *entt;
  if ((entt = malloc(sizeof(*entt))) == NULL) return NULL;
  if ((entt->components = short_vec_make(sizeof(gecs_component_t *), 1)) ==
      NULL)
    return NULL;
  entt->id = world->id_gen++;
  entt->is_alive = true;

  short_vec_push(world->entities, entt);

  // We free entt and not components because short_vec_push will do a shallow
  // copy of the memory of the struct but not the pointers pointing outside
  // the struct
  free(entt);

  return entt;
}

int gecs_add_component(gecs_core_t *world, gecs_entity_t *entt,
                       const char *component_name, const size_t len) {
  int         itr_len, itr;
  gecs_size_t name_hash;

  bool              found_in_list;
  gecs_component_t *component, construct;

  name_hash = name_to_id(component_name, len);

  // Check to make sure the component has not already been added to the entity

  found_in_list = false;
  itr_len = short_vec_len(entt->components);
  for (itr = 0; itr < itr_len; itr++) {
    component = (gecs_component_t *)short_vec_at(entt->components, itr);
    if (component->name_hash == name_hash) {
      found_in_list = true;
      break;
    }
  }
  if (found_in_list) return GECS_FAIL;

  // Check to make sure the component does exist within the registry
  found_in_list = false;
  itr_len = short_vec_len(world->component_registry);
  for (itr = 0; itr < itr_len; itr++) {
    component =
        (gecs_component_t *)short_vec_at(world->component_registry, itr);

    if (component->name_hash == name_hash) {
      found_in_list = true;
      break;
    }
  }
  if (!found_in_list) return GECS_FAIL;

  // Add component now
  construct.component_size = component->component_size;
  construct.name_hash = name_hash;
  if ((construct.component = malloc(construct.component_size)) == NULL)
    return GECS_FAIL;

  short_vec_push(entt->components, &construct);
  return GECS_OK;
}

static const gecs_size_t name_to_id(const char *str, const size_t len) {
  char *hash_str;
  if ((hash_str = malloc(sizeof(*hash_str) * len + 1)) == NULL)
    return GECS_FAIL;
  memcpy(hash_str, str, len);
  hash_str[len] = '\0';
  free(hash_str);
  return hash(hash_str);
}

static const bool world_has_component(const gecs_core_t *world, char *name,
                                      size_t len) {
  int         reg_len, itr;
  gecs_size_t comp_id;

  comp_id = name_to_id(name, len);

  reg_len = short_vec_len(world->component_registry);
  for (itr = 0; itr < reg_len; itr++) {
    if (((gecs_component_t *)short_vec_at(world->component_registry, itr))
            ->name_hash == comp_id)
      return true;
  }

  return false;
}