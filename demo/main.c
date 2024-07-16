#include "csdsa.h"
#include <stdlib.h>

#define ID_IS_ENTITY    0
#define ID_IS_COMPONENT 1

/* Component ID -> Index in related component vector */
MAP_TYPEDEC(component_indexer, int32_t, int32_t);
MAP_TYPE_IMPL(component_indexer, int32_t, int32_t);

/* Entity ID -> Component Indexer */
MAP_TYPEDEC(entity_map, int32_t, component_indexer);
MAP_TYPE_IMPL(entity_map, int32_t, component_indexer);

/* Component ID -> Vector of component */
MAP_TYPEDEC(component_datamap, int32_t, vec);
MAP_TYPE_IMPL(component_datamap, int32_t, vec);

VEC_TYPEDEC(int32_vec, int32_t);
VEC_TYPE_IMPL(int32_vec, int32_t);

typedef void (*sys)(vec *components);

typedef struct ecs_sys ecs_sys;
struct ecs_sys {
  sys       ecs_system;   /* User defined function. */
  int32_vec requirements; /* List of component IDs */
};

VEC_TYPEDEC(ecs_systems, ecs_sys);
VEC_TYPE_IMPL(ecs_systems, ecs_sys);

typedef struct ecs ecs;
struct ecs {
  int32_t           id_gen;     /* Generates unique IDs for the ECS demo */
  entity_map        entities;   /* Contains all entity info */
  component_datamap components; /* Contains all component data */
  ecs_systems       systems;    /* Vector of all the systems */
  stalloc          *alloc;      /* The allocator for this ECS */
};

static component_indexer *load_index_from_entity(ecs *e, int32_t entt) {
  kvpair entity_kv = entity_map_get(&e->entities, &entt);
  assert(entity_kv.value);
  return entity_kv.value;
}

static vec *load_component_vector_from_id(ecs *e, int32_t comp_id) {
  kvpair component_vec_kv = component_datamap_get(&e->components, &comp_id);
  assert(component_vec_kv.value);
  return component_vec_kv.value;
}

ecs *ecs_new(void) {
  ecs *e = calloc(1, sizeof(ecs));

  e->id_gen = 0;
  e->alloc = stalloc_create(1024);
  start_frame(e->alloc);

  entity_map_hinit(&e->entities);
  component_datamap_hinit(&e->components);
  ecs_systems_hinit(&e->systems);

  return e;
}

void ecs_progress(ecs *e) {
  for (int64_t sys_i = 0; sys_i < e->systems.length; sys_i++) {
    ecs_sys *sys = ecs_systems_at(&e->systems, sys_i);

    if (sys->requirements.length == 0) {
      sys->ecs_system(NULL);
      continue;
    }

    for (int64_t comp_i = 0; comp_i < sys->requirements.length; comp_i++) {
      int64_t *comp_id = vec_at(&sys->requirements, comp_i);
      sys->ecs_system(load_component_vector_from_id(e, *comp_id));
    }
  }
}

void ecs_free(ecs *e) {
  entity_map_free(&e->entities);
  component_datamap_free(&e->components);
  ecs_systems_hinit(&e->systems);
  end_frame(e->alloc);
  free(e);
}

int32_t ecs_id(ecs *e, int32_t FLAGS) {
  int32_t new_id = e->id_gen++;

  if (FLAGS == ID_IS_ENTITY) {
    component_indexer new_indexer;
    component_indexer_hinit(&new_indexer);
    entity_map_put(&e->entities, &new_id, &new_indexer);
  }
  return new_id;
}

void register_system(ecs *e, int32_vec *requirements, sys system_func) {
  ecs_sys new_system = {0};
  new_system.ecs_system = system_func;
  int32_vec_copy(&new_system.requirements, requirements);
  ecs_systems_push(&e->systems, &new_system);
}

void register_component(ecs *e, int32_t comp_id, size_t component_size) {
  /* Allocate 64 elements because why not. Sounds reasonable. */
  vec component_vector;
  __vec_init(&component_vector, component_size, get_frame_ctx(), TO_HEAP, 64);
  component_datamap_put(&e->components, &comp_id, &component_vector);
}

void ecs_entt_add_component(ecs *e, int32_t entt, int32_t comp_id) {
  vec *component_vector = load_component_vector_from_id(e, comp_id);

  /* Place at the end of the list and store the position */
  int64_t pos = component_vector->length;
  vec_resize(component_vector, pos + 1);

  /* Store the index the component lives at in the component vector. */
  component_indexer_put(load_index_from_entity(e, entt), &comp_id, &pos);

  /* Set the memory at this position to zero for convenience */
  memset(vec_at(component_vector, pos), 0, component_vector->__el_size);
}

void ecs_entt_rem_component(ecs *e, int32_t entt, int32_t comp_id) {
  /* Delete the component position for the entity*/
  component_indexer_del(load_index_from_entity(e, entt), &comp_id);
}
void *ecs_entt_get_component(ecs *e, int32_t entt, int32_t comp_id) {
  vec     *component_vector = load_component_vector_from_id(e, comp_id);
  int32_t *pos =
      component_indexer_get(load_index_from_entity(e, entt), &comp_id).value;

  if(pos == NULL) return NULL;
  return vec_at(component_vector, *pos);
}

typedef struct Vec2 Vec2;
struct Vec2 {
  int16_t x, y;
};

ecs    *e;
int32_t PLAYER, FRIEND;
int32_t VEC2_ID;
bool    simulation_running = true;

void gravity(vec *positions) {
  for (int64_t i = 0; i < positions->length; i++) {
    Vec2 *pos = vec_at(positions, i);
    pos->y--;
  }
}

void simulation_end_check() {
  Vec2 *pos = ecs_entt_get_component(e, PLAYER, VEC2_ID);
  if (pos && pos->y <= 0) {
    printf("Player hit the ground!\n");
    ecs_entt_rem_component(e, PLAYER, VEC2_ID);
  } else
    return;

  pos = ecs_entt_get_component(e, FRIEND, VEC2_ID);
  if (pos && pos->y <= 0) {
    printf("Friend hit the ground!\n");
    ecs_entt_rem_component(e, FRIEND, VEC2_ID);
  } else
    return;

  simulation_running = false;
}

int main(void) {
  e = ecs_new();

  /* Register IDs */
  PLAYER = ecs_id(e, ID_IS_ENTITY);
  FRIEND = ecs_id(e, ID_IS_ENTITY);
  VEC2_ID = ecs_id(e, ID_IS_COMPONENT);

  /* Register Components */
  register_component(e, VEC2_ID, sizeof(Vec2));

  /* Register Systems */
  int32_vec requirements;
  int32_vec_hinit(&requirements);

  register_system(e, &requirements, simulation_end_check);
  int32_vec_push(&requirements, &VEC2_ID);
  register_system(e, &requirements, gravity);

  /* Populate PLAYER and FRIEND with the Vec2 Component */
  ecs_entt_add_component(e, PLAYER, VEC2_ID);
  ecs_entt_add_component(e, FRIEND, VEC2_ID);

  /* Set the entities positions */
  ((Vec2 *)ecs_entt_get_component(e, PLAYER, VEC2_ID))->y = 12;
  ((Vec2 *)ecs_entt_get_component(e, FRIEND, VEC2_ID))->y = 10;

  while (simulation_running) ecs_progress(e);

  printf("Simulation Done\n");

  ecs_free(e);
  return 0;
}