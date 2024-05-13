#include "gecs.h"
#include <stdio.h>

/*-------------------------------------------------------
 * STRUCTS AND TYPE DEFINITIONS
 *-------------------------------------------------------*/

struct gecs_entity_t {
  short_vec_t *components; /* of gecs_component_t */
  bool         is_alive;
  gecs_size_t  id;
};

struct gecs_core_t {
  gecs_size_t  id_gen;
  short_vec_t *systems;            /* of gecs_system_t */
  short_vec_t *entities;           /* of gecs_entity_t */
  short_vec_t *component_registry; /* of component_t */
};

struct gecs_q_t {
  bool is_init;

  gecs_core_t *world;
  short_vec_t *components; /* gecs_size_t */
};

typedef struct gecs_component_t gecs_component_t;
struct gecs_component_t {
  gecs_size_t name_hash;
  size_t      component_size;
  void       *component;
};

typedef struct gecs_system_t gecs_system_t;
struct gecs_system_t {
  void (*run_sys)(gecs_q_t *q);
  gecs_q_t q;
};

/*-------------------------------------------------------
 * STATIC FUNCTION DEFINITIONS
 *-------------------------------------------------------*/

static const gecs_size_t name_to_id(const char *str, const size_t len);
static const bool     world_has_component(const gecs_core_t *world, char *name,
                                          size_t len);
static gecs_entity_t *gecs_id_to_entt(gecs_core_t      *world,
                                      const gecs_size_t gec_id);

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
  int itr_sys, sys_cnt;

  gecs_system_t *sys;

  sys_cnt = short_vec_len(world->systems);
  for (itr_sys = 0; itr_sys < sys_cnt; itr_sys++) {
    sys = (gecs_system_t *)short_vec_at(world->systems, itr_sys);
    sys->run_sys(&sys->q);
  }
}

void gecs_complete(gecs_core_t *world) {
  int               itr, comp_itr;
  gecs_system_t    *sys;
  gecs_entity_t    *entt;
  gecs_component_t *comp;

  // Free the components list of each entity
  for (itr = 0; itr < short_vec_len(world->entities); itr++) {
    entt = (gecs_entity_t *)short_vec_at(world->entities, itr);
    for (comp_itr = 0; comp_itr < short_vec_len(entt->components); comp_itr++) {
      comp = (gecs_component_t *)short_vec_at(entt->components, comp_itr);
      free(comp->component);
    }
    short_vec_free(entt->components);
  }

  // Free the entity list
  short_vec_free(world->entities);

  // Free the system info of each system
  for (itr = 0; itr < short_vec_len(world->systems); itr++) {
    sys = (gecs_system_t *)short_vec_at(world->systems, itr);
    short_vec_free(sys->q.components);
  }
  short_vec_free(world->systems);
  short_vec_free(world->component_registry);

  free(world);
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

int gecs_register_system(gecs_core_t *world, void (*sys)(gecs_q_t *q)) {
  gecs_system_t gec_sys;

  gec_sys.q.components = short_vec_make(sizeof(gecs_size_t), 1);
  gec_sys.q.is_init = false;
  gec_sys.q.world = world;

  gec_sys.run_sys = sys;

  short_vec_push(world->systems, &gec_sys);

  return GECS_OK;
}

/*-------------------------------------------------------
 * ENTITY MANIPULATION FUNCTIONS
 *-------------------------------------------------------*/

gecs_size_t gecs_make_entity(gecs_core_t *world) {
  short_vec_push(world->entities,
                 &(gecs_entity_t){.id = world->id_gen++,
                                  .is_alive = true,
                                  .components = short_vec_make(
                                      sizeof(gecs_component_t), 1)});

  gecs_entity_t *entt = (gecs_entity_t *)short_vec_top(world->entities);
  return entt->id;
}

int gecs_add_component(gecs_core_t *world, gecs_size_t entt_id,
                       const char *component_name, const size_t len) {
  int         itr_len, itr;
  gecs_size_t name_hash;

  bool              found_in_list;
  gecs_component_t *component;

  gecs_entity_t *entt;

  name_hash = name_to_id(component_name, len);
  entt = gecs_id_to_entt(world, entt_id);

  if (entt == NULL) return GECS_FAIL;

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

  short_vec_push(
      entt->components,
      &(gecs_component_t){.name_hash = name_hash,
                          .component_size = component->component_size,
                          .component = malloc(component->component_size)});

  return GECS_OK;
}

int gecs_q_define(gecs_q_t *q, char *component_names[]) {
  if (q->is_init) return GECS_OK;

  int         i;
  gecs_size_t name_hash;
  for (i = 0; component_names[i] != NULL; i++) {
    name_hash = name_to_id(component_names[i], strlen(component_names[i]));
    short_vec_push(q->components, &name_hash);
  }

  q->is_init = true;
  return GECS_OK;
}

short_vec_t *gecs_q_select_sub_entities(gecs_q_t *q) {
  short_vec_t *output;

  bool matches;

  gecs_entity_t    *entt;
  gecs_component_t *comp, *q_comp;
  int cnt_entt, itr_entt, cnt_comp, itr_comp, cnt_q_comp, itr_q_comp;

  output = short_vec_make(sizeof(gecs_entity_t *), 1);

  cnt_entt = short_vec_len(q->world->entities);
  cnt_q_comp = short_vec_len(q->components);
  for (itr_entt = 0; itr_entt < cnt_entt; itr_entt++) {
    entt = (gecs_entity_t *)short_vec_at(q->world->entities, itr_entt);

    matches = false;
    cnt_comp = short_vec_len(entt->components);
    for (itr_comp = 0; itr_comp < cnt_comp; itr_comp++) {
      comp = (gecs_component_t *)short_vec_at(entt->components, itr_comp);

      for (itr_q_comp = 0; itr_q_comp < cnt_q_comp; itr_q_comp++) {
        q_comp = (gecs_component_t *)short_vec_at(q->components, itr_q_comp);

        if (q_comp->name_hash == comp->name_hash) {
          short_vec_push(output, entt);
          matches = true;
          break;
        }
      }
      if (matches) break;
    }
  }

  return output;
}

void *gecs_entity_get_by_id(gecs_core_t *world, gecs_size_t id, char *name) {
  return gecs_entity_get(gecs_id_to_entt(world, id), name);
}

void *gecs_entity_get(gecs_entity_t *entt, char *name) {
  gecs_size_t name_hash = name_to_id(name, strlen(name));

  gecs_component_t *comp;
  int               cnt_comp, itr_comp;
  cnt_comp = short_vec_len(entt->components);
  for (itr_comp = 0; itr_comp < cnt_comp; itr_comp++) {
    comp = (gecs_component_t *)short_vec_at(entt->components, itr_comp);
    if (comp->name_hash == name_hash) {
      return comp->component;
    }
  }
  return NULL;
}

static gecs_entity_t *gecs_id_to_entt(gecs_core_t      *world,
                                      const gecs_size_t gec_id) {
  int            entt_cnt, itr_entt;
  gecs_entity_t *entt;

  entt_cnt = short_vec_len(world->entities);
  for (itr_entt = 0; itr_entt < entt_cnt; itr_entt++) {
    entt = short_vec_at(world->entities, itr_entt);
    if (entt->id == gec_id) return entt;
  }
  return NULL;
}

static const gecs_size_t name_to_id(const char *str, const size_t len) {
  char *hash_str;
  if ((hash_str = malloc(sizeof(*hash_str) * len + 1)) == NULL)
    return GECS_FAIL;
  memcpy(hash_str, str, len);
  hash_str[len] = '\0';
  gecs_size_t x = hash(hash_str);
  free(hash_str);
  return x;
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