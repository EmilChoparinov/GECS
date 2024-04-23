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
};

typedef struct gecs_system_t gecs_system_t;
struct gecs_system_t {
  gecs_system_info_t *info;
  short_vec_t        *components_to_query; // of int id's
  gecs_size_t         id;
};

struct gecs_core_t {
  gecs_size_t  id_gen;
  short_vec_t *systems;            // of gecs_system_t
  short_vec_t *entities;           // of gecs_entity_t
  short_vec_t *component_registry; // of component_t
};

static const gecs_size_t name_to_id(const char *str, const size_t len);

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
  for (itr = 0; itr < short_vec_len(world->systems); itr++)
    sys = (gecs_system_t *)(world->systems, itr);
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
    free(sys->info);
  }
  short_vec_free(world->systems);
}

int gecs_register_component(gecs_core_t                  *world,
                            struct gecs_component_info_t *info) {
  char            *hash_name;
  gecs_component_t comp;

  comp.component_size = info->component_size;
  comp.id = world->id_gen++;
  comp.name_hash = name_to_id(info->name, info->name_len);

  short_vec_push(world->component_registry, &comp);
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