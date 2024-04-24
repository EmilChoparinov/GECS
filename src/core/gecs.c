#include "gecs.h"
struct gecs_entity_t {
  short_vec_t *components; // of pointers to declared components
  bool         is_alive;
  gecs_size_t  id;
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
  short_vec_t *components_to_query; // of gecs_size_t id's
};

struct gecs_core_t {
  gecs_size_t  id_gen;
  short_vec_t *systems;            // of gecs_system_t
  short_vec_t *entities;           // of gecs_entity_t
  short_vec_t *component_registry; // of component_t
};

static const gecs_size_t name_to_id(const char *str, const size_t len);

// Comparators
bool already_added(void *el) { return true; }

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
  int            itr;
  gecs_system_t *sys;
  for (itr = 0; itr < short_vec_len(world->systems); itr++) {
    sys = (gecs_system_t *)short_vec_at(world->systems, itr);
    sys->run_sys(NULL);
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

int gecs_register_component(gecs_core_t *world, gecs_component_info_t *info) {
  int         reg_len, itr;
  gecs_size_t name_to_add;

  name_to_add = name_to_id(info->name, info->name_len);

  // Check to make sure there exists no other component with the same name/id.
  reg_len = short_vec_len(world->component_registry);
  for (itr = 0; itr < reg_len; itr++) {
    if (((gecs_component_t *)short_vec_at(world->component_registry, itr))
            ->name_hash == name_to_add)
      return GECS_FAIL;
  }

  short_vec_push(world->component_registry,
                 &(gecs_component_t){.component_size = info->component_size,
                                     .name_hash = name_to_add,
                                     .component = NULL});
  return GECS_OK;
}

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
  // copy of the memory of the struct but not the pointers pointing outside the
  // struct
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

// int gecs_register_system(gecs_core_t *world, gecs_system_info_t *info) {}

static const gecs_size_t name_to_id(const char *str, const size_t len) {
  char *hash_str;
  if ((hash_str = malloc(sizeof(*hash_str) * len + 1)) == NULL)
    return GECS_FAIL;
  memcpy(hash_str, str, len);
  hash_str[len] = '\0';
  free(hash_str);
  return hash(hash_str);
}