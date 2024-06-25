#include <pthread.h>

#include "gecs.h"

typedef struct entity_record entity_record;
typedef entity_record       *pentity_record;
typedef struct system_data   system_data;
typedef system_data         *psystem_data;
struct system_data {
  g_system  run_system;   /* A function pointer to the system. */
  gid_vec_t requirements; /* String of components in the form "A,B,C". */
};
VECTOR_GEN_H(system_data);
VECTOR_GEN_C(system_data);

VECTOR_GEN_H(psystem_data);
VECTOR_GEN_C(psystem_data);

MAP_GEN_H(gid, pentity_record);
MAP_GEN_C(gid, pentity_record);

struct archetype {
  gid archetype_id; /* Unique Identifier for this archetype. */

  gid_vec_t          type_set;   /* Ordered Vec: hash(names) */
  fragment_vec_t     composite;  /* Contiguous vector of interleaved compents */
  psystem_data_vec_t contenders; /* Vec: system_data* */

  gid_gsize_map_t offsets; /* Map: hash(name) -> interleaved comp offset */

  /* The following members are made for concurrency and caching purposes. */
  g_core_t  *cache;               /* OOB mutations go here. */
  gid_vec_t  entt_delete_queue;   /* Queue : entt id */
  gid_vec_t  entt_creation_queue; /* Queue : c(entt id) */
  gint_vec_t dead_fragments;      /* Ordered Vec : index_of(composite) */

  gid_gid_map_t            cache_interface; /* Map: entt id -> c(record) */
  gid_pentity_record_map_t entt_members;    /* Map : entt id -> *record */
};
MAP_GEN_H(gid, archetype);
MAP_GEN_C(gid, archetype);

struct entity_record {
  archetype *a;     /* The archetype this entity belongs to. */
  gint       index; /* The index to collect their components. */
};

MAP_GEN_H(gid, entity_record);
MAP_GEN_C(gid, entity_record);

struct g_query_t {
  g_core_t  *world;                /* The world being queried on. */
  archetype *archetype_context;    /* Which archetype this querys context is. */
  fragment_vec_t *component_store; /* Pointer to composite */
};

struct g_itr {
  fragment_vec_t  *stored_components;
  gid_gsize_map_t *offsets;
};

struct g_core_t {
  gid    id_gen;        /* Generates unique IDs */
  int8_t reprocess_fsm; /* When this flag is set to 1. It will reprocess the
                           FSM before next tick. */
  int8_t is_main;       /* When this flag is set to 1. It represents that this
                           world contains storage. */

  /* These are all gid maps because the hashing function outputs the same size
     as gid, so it's convenient. */
  gid_gsize_map_t     component_registry;  /* Map: hash(name) -> comp size */
  gid_archetype_map_t archetype_registry;  /* Map: hash([name]) -> archetype */
  gid_entity_record_map_t entity_registry; /* Map: entt id -> entt record */

  system_data_vec_t system_registry; /* Vec: system_data. */
};

/*-------------------------------------------------------
 * Static Forward Declarations
 *-------------------------------------------------------*/
static retcode _g_init_archetype(g_core_t *w, archetype *a, gid_vec_t *types);
static retcode _g_transfer_archetypes(g_core_t *w, gid entt, gid_vec_t *types);
static retcode _g_archetype_key(char *types, gid_vec_t *hashes);
static retcode _g_assign_fsm(g_core_t *w);
static retcode _g_migrate(g_core_t *w);
static retcode _g_cleanup(g_core_t *w);
static void    _g_defragment(g_core_t *w);
static gid     _g_create_cache_interface(archetype *a, gid entt);
static gid     _g_access_cache_entt(archetype *a, gid entt);
static gid     hash_vec(any_vec_t *v);

/*-------------------------------------------------------
 * Globals
 *-------------------------------------------------------*/
/* We define the empty archetype to be an arbitrary unique address.*/
static archetype empty_archetype = {0};

g_core_t *g_create_world(void) {
  log_debug("new world created\n");
  g_core_t *w = malloc(sizeof(g_core_t));

  w->is_main = 1;
  w->id_gen = 0;
  GID_SET_MODE(w->id_gen, STORAGE);
  w->reprocess_fsm = 1;

  gid_gsize_map_init(&w->component_registry);
  gid_entity_record_map_init(&w->entity_registry);
  gid_archetype_map_init(&w->archetype_registry);

  system_data_vec_init(&w->system_registry);

  /* Curiously, I decided to not put the empty archetype into the registry.
     Why? I don't want to risk adding an extra coll;ision when querying. This if
     ok because the empty archetype represents garbage so a user will never
     query for it. */

  return w;
}

static retcode _g_destroy_world(g_core_t *w);
static bool    f_free_archetype(gid_archetype_map_item *it, void *arg) {
  archetype *a = &it->value;
  gid_vec_free(&a->type_set);
  fragment_vec_free(&a->composite);
  psystem_data_vec_free(&a->contenders);

  gid_gsize_map_free(&a->offsets);

  if (a->cache) _g_destroy_world(a->cache);

  gid_vec_free(&a->entt_delete_queue);
  gid_vec_free(&a->entt_creation_queue);

  gint_vec_free(&a->dead_fragments);

  gid_gid_map_free(&a->cache_interface);
  gid_pentity_record_map_free(&a->entt_members);

  return true;
}

static bool f_free_system_data(system_data *sd, void *arg) {
  gid_vec_free(&sd->requirements);
  return true;
}

static retcode _g_destroy_world(g_core_t *w) {
  gid_gsize_map_free(&w->component_registry);
  gid_entity_record_map_free(&w->entity_registry);
  gid_archetype_map_foreach(&w->archetype_registry, f_free_archetype, NULL);
  gid_archetype_map_free(&w->archetype_registry);

  system_data_vec_foreach(&w->system_registry, f_free_system_data, NULL);
  system_data_vec_free(&w->system_registry);

  free(w);

  return R_OKAY;
}

retcode g_destroy_world(g_core_t *w) {
  log_debug("destroying\n");
  return _g_destroy_world(w);
}

bool run_sys_process(void **arch, g_core_t *world) {
  archetype *a = *arch;
  for (size_t sys_i = 0; sys_i < a->contenders.length; sys_i++) {
    system_data *sd = *psystem_data_vec_at(&a->contenders, sys_i);
    sd->run_system(&(g_query_t){.world = world,
                                .archetype_context = a,
                                .component_store = &a->composite});
  }
  return true;
}

void *init_thread(void *arg) {
  log_debug("THREAD START\n");
  void **args = (void **)arg;
  run_sys_process(args[0], args[1]);
  log_debug("THREAD STOP\n");
  return NULL;
}

retcode g_progress(g_core_t *w) {
  log_debug("--- WORLD TICK START\n");
  if (w->reprocess_fsm) _g_assign_fsm(w);

  /* We run each system */
  any_vec_t arch_vec;
  gid_archetype_map_ptrs(&w->archetype_registry, &arch_vec);

  pthread_t *threads = malloc(arch_vec.length * sizeof(pthread_t));

  any_vec_t arg_list;
  vec_unknown_type_init(&arg_list, sizeof(void *));

  for (size_t i = 0; i < arch_vec.length; i++) {
    void **args = malloc(2 * sizeof(void *));
    args[0] = any_vec_at(&arch_vec, i);
    args[1] = w;

    any_vec_push(&arg_list, of_any(args));
    /* If the composite is empty, then there are no entities here therefore do
       not run the system */
    if ((*(archetype **)any_vec_at(&arch_vec, i))->composite.length == 0) {
      log_warn("Archetype %ld skipped since it contains no entities\n",
               ((archetype *)args[0])->archetype_id);
    }
    log_trace("started thread id: %ld\n", i);
    pthread_create(&threads[i], NULL, init_thread, args);
  };

  for (size_t i = 0; i < arch_vec.length; i++) {
    log_debug("waiting for thread %ld to finish\n", i);
    if (pthread_join(threads[i], NULL)) {
      log_debug("Threads unable to join!");
      exit(EXIT_FAILURE);
    }
  }

  /* Migrate changes saved in the ledger, then clean the caches for next itr */
  _g_migrate(w);
  _g_cleanup(w);
  _g_defragment(w);

  for (size_t i = 0; i < arg_list.length; i++) free(*any_vec_at(&arg_list, i));

  free(threads);
  any_vec_free(&arg_list);
  any_vec_free(&arch_vec);

  log_debug("--- WORLD TICK END\n");
  return R_OKAY;
}

retcode g_register_component(g_core_t *w, char *name, size_t component_size) {
  gid hash_name = (gid)hash_bytes(name, strlen(name));
  log_info("Registered component {name: %s, id: %ld}\n", name, hash_name);
  return gid_gsize_map_put(&w->component_registry, &hash_name, &component_size);
}

retcode g_register_system(g_core_t *w, g_system sys, char *query) {
  assert(query);
  assert(sys);

  gid_vec_t type_set;
  _g_archetype_key(query, &type_set);

  return system_data_vec_push(
      &w->system_registry,
      &(system_data){.requirements = type_set, .run_system = sys});
}

static retcode type_union(gid_vec_t *a, gid_vec_t *b, gid_vec_t *out) {
  gid_gid_map_t type_set;
  gid_gid_map_init(&type_set);

  // Add old
  for (size_t i = 0; i < a->length; i++) {
    gid p = *gid_vec_at(a, i);
    gid_gid_map_put(&type_set, &p, &p);
  }

  // Add new
  for (size_t i = 0; i < b->length; i++) {
    gid p = *gid_vec_at(b, i);
    gid_gid_map_put(&type_set, &p, &p);
  }

  gid_gid_map_to_vec(&type_set, (any_vec_t *)out);
  gid_gid_map_free(&type_set);
  return R_OKAY;
}

static retcode __g_add_component(g_core_t *w, gid entt_id,
                                 gid_vec_t *new_types) {
  gid_entity_record_map_item *item =
      gid_entity_record_map_find(&w->entity_registry, &entt_id);
  assert(item && "Entity does not exist!");

  gid_vec_t *old_types = &item->value.a->type_set;
  assert(!vectors_intersect(old_types, new_types) &&
         "Some components already exist on entity!");

  gid_vec_t archetype_set;
  type_union(new_types, old_types, &archetype_set);

  _g_transfer_archetypes(w, entt_id, &archetype_set);

  gid_vec_free(new_types);
  gid_vec_free(&archetype_set);

  return R_OKAY;
}

retcode g_add_component(g_core_t *w, gid entt_id, char *name) {
  log_info("add component {id: %ld, name: %s, ctx: %d }\n", entt_id, name,
           SELECT_MODE(entt_id));
  gid_vec_t new_types;
  _g_archetype_key(name, &new_types);

  return __g_add_component(w, entt_id, &new_types);
}

static void *__g_get_component(g_core_t *w, gid entt_id, gid type) {
  gid_entity_record_map_item *item =
      gid_entity_record_map_find(&w->entity_registry, &entt_id);
  assert(item && "Entity does not exist!");

  entity_record entt_rec = item->value;

  gid_gsize_map_item *typekv = gid_gsize_map_find(&entt_rec.a->offsets, &type);
  assert(typekv && "Component not registered!");
  gsize comp_off = typekv->value;

  buff_t buff;
  buff_init(&buff, fragment_vec_at(&entt_rec.a->composite, entt_rec.index));

  return buff_skip(&buff, comp_off);
}

void *g_get_component(g_core_t *w, gid entt_id, char *name) {
  log_info("get component {id: %ld, name: %s, ctx:%d }\n", entt_id, name,
           SELECT_MODE(entt_id));
  return __g_get_component(w, entt_id, (gid)hash_bytes(name, strlen(name)));
}

static retcode __g_set_component(g_core_t *w, gid entt_id, gid type,
                                 void *comp) {
  /* Load entity */
  gid_entity_record_map_item *entt_item =
      gid_entity_record_map_find(&w->entity_registry, &entt_id);
  assert(entt_item && "Entity does not exist!");

  entity_record entt_rec = entt_item->value;

  /* Load component offset from archetype entity is member of */
  gid_gsize_map_item *type_item =
      gid_gsize_map_find(&entt_rec.a->offsets, &type);
  assert(type_item && "Component not registered!");

  gsize comp_off = type_item->value;

  buff_t buff;
  buff_init(&buff, fragment_vec_at(&entt_rec.a->composite, entt_rec.index));

  /* Copy data into composite */
  memmove(buff_skip(&buff, comp_off), comp, entt_rec.a->composite.__el_size);
  return R_OKAY;
}

retcode g_set_component(g_core_t *w, gid entt_id, char *name, void *comp) {
  log_info("set component {id: %ld, name: %s, ctx:%d }\n", entt_id, name,
           SELECT_MODE(entt_id));
  gid type = (gid)hash_bytes(name, strlen(name));
  return __g_set_component(w, entt_id, type, comp);
}

retcode g_rem_component(g_core_t *w, gid entt_id, char *name) {
  log_info("rem component {id: %ld, name: %s, ctx:%d }\n", entt_id, name,
           SELECT_MODE(entt_id));
  gid_entity_record_map_item *entt =
      gid_entity_record_map_find(&w->entity_registry, &entt_id);
  assert(entt && "Entity does not exist!");

  /* Collect component ids from the query */
  gid_vec_t new_types;
  _g_archetype_key(name, &new_types);

  /* Collect component ids from the archetype */
  gid_vec_t *old_types = &entt->value.a->type_set;

  assert(vectors_intersect(&new_types, old_types) &&
         "Remove contains components already not on entity!");

  /* Perform the difference old_types / new_types */
  gid_vec_t type_set;
  gid_vec_copy(gid_vec_init(&type_set), old_types);

  for (int64_t type_i = 0; type_i < type_set.length; type_i++) {
    gid *cur_type = gid_vec_at(&type_set, type_i);
    if (gid_vec_has(&new_types, cur_type)) gid_vec_delete(&type_set, cur_type);
  }

  retcode code = _g_transfer_archetypes(w, entt_id, &type_set);

  gid_vec_free(&type_set);
  gid_vec_free(&new_types);
  return code;
}

static bool f_reset_archetypes(gid_archetype_map_item *item, void *arg) {
  psystem_data_vec_clear(&item->value.contenders);
  return true;
}

static retcode _g_assign_fsm(g_core_t *w) {
  /* Clear the old system positions. */
  gid_archetype_map_foreach(&w->archetype_registry, f_reset_archetypes, NULL);

  /* For each archetype, we apply systems to the contend list that intersect on
     types */
  any_vec_t archetypes;
  gid_archetype_mmap_to_vec(&w->archetype_registry, &archetypes);

  for (size_t arch_i = 0; arch_i < archetypes.length; arch_i++) {
    /* archetype** because map_to_vec returns an array of addresses to
       the element location in the map. */
    archetype *arch = *(archetype **)any_vec_at(&archetypes, arch_i);

    for (size_t sys_i = 0; sys_i < w->system_registry.length; sys_i++) {
      system_data *sd = system_data_vec_at(&w->system_registry, sys_i);
      if (vectors_intersect(&arch->type_set, &sd->requirements)) {
        psystem_data_vec_push(&arch->contenders, &sd);
      }
    }
  }

  w->reprocess_fsm = 0; /* Set process flag as complete. */
  any_vec_free(&archetypes);

  return R_OKAY;

  /* For each system */
  for (size_t sys_i = 0; sys_i < w->system_registry.length; sys_i++) {
    system_data *sd = system_data_vec_at(&w->system_registry, sys_i);

    /* Get archetypes */
    any_vec_t arch_vec;
    gid_archetype_map_ptrs(&w->archetype_registry, &arch_vec);

    /* For each archetype */
    for (size_t arch_i = 0; arch_i < arch_vec.length; arch_i++) {
      archetype *arch = *(archetype **)any_vec_at(&arch_vec, arch_i);

      /* Check if archetype intersects. If so push and break. */
      for (size_t type_i = 0; type_i < sd->requirements.length; type_i++) {
        gid *type = gid_vec_at(&sd->requirements, type_i);

        if (!gid_vec_has(&arch->type_set, type)) continue;

        psystem_data_vec_push(&arch->contenders, &sd);
        break;
      }
    }
  }

  w->reprocess_fsm = 0; /* Set process flag as done with reprocessing. */

  return R_OKAY;
}

static retcode _g_archetype_key(char *types, gid_vec_t *hashes) {
  assert(types);
  /* Since archetypes are sets, and query strings can be in the various forms
     such as "A,B,C" or "B,A,C", we need a mechanism that maps all string
     combinations into a set. We perform a sort operation with a stack vector
     over the hash of each individual component. This allows us to rehash the
     normalized vector and produce a key that works for all iterations of
     "A,B,C".

     Collision Detection:
     Since we are using hashing algorithms here we must be careful with
     collisions. The way we protect against collisions is by caching the
     archetype sort vector and doing a memory check if they are the same. If
     a hash leads to an archetype that is **not** the archetype queried for,
     the library will assert and exit.
     */

  /* The group size for the regex matcher is the count of ',' in the string.
     Therefore we interate and count how many there are. */
  gint   archetype_len = 1;
  size_t i, str_len = strlen(types);
  for (i = 0; i < str_len && i < GECS_MAX_ARCHETYPE_COMPOSITION; i++)
    if (types[i] == ',') archetype_len++;
  assert(i != GECS_MAX_ARCHETYPE_COMPOSITION &&
         "Compositions allowed up to 256!");

  gid_vec_init(hashes);

  regex_t    matcher;
  regmatch_t groups[GECS_MAX_ARCHETYPE_COMPOSITION];
  regcomp(&matcher, "(\\w+)", REG_EXTENDED);

  /* Assert that the query actually got matches. */
  assert(regexec(&matcher, types, GECS_MAX_ARCHETYPE_COMPOSITION, groups, 0) !=
         REG_NOMATCH);

  for (size_t i = 0; i < archetype_len; i++) {
    /* groups[0] contains the full matched string, not groups. */
    regoff_t arg_start = groups[i + 1].rm_so;
    regoff_t arg_end = groups[i + 1].rm_eo;

    gid component_id = (gid)hash_bytes(&types[arg_start], arg_end - arg_start);

    if (hashes->length == 0) {
      gid_vec_push(hashes, &component_id);
      continue;
    }

    if (*gid_vec_top(hashes) < component_id) {
      gid *old_top = gid_vec_top(hashes);
      gid_vec_pop(hashes);
      gid_vec_push(hashes, &component_id);
      gid_vec_push(hashes, old_top);
      continue;
    }

    gid_vec_push(hashes, &component_id);
  }

  regfree(&matcher);

  return R_OKAY;
};

gid g_create_entity(g_core_t *w) {
  gid id = w->id_gen;
  GID_INCR(w->id_gen);

  /* All entities start initially at the empty archetype */
  gid_entity_record_map_put(
      &w->entity_registry, &id,
      &(entity_record){.a = &empty_archetype, .index = -1});

  return id;
}

bool gq_id_in(g_query_t *q, gid id) {
  log_debug("id in check: {id: %d}\n", id);
  bool in_storage =
      gid_pentity_record_map_has(&q->archetype_context->entt_members, &id);
  if (in_storage) return true;

  return gid_gid_map_has(&q->archetype_context->cache_interface, &id);
}

retcode g_queue_delete(g_core_t *w, gid entt) {
  log_debug("entt queued to delete: {id: %ld, ctx: %d}", entt,
            SELECT_MODE(entt));
  /* Entity deletions also interface into the cache world */
  gid_entity_record_map_item *entt_item =
      gid_entity_record_map_find(&w->entity_registry, &entt);
  assert(entt_item && "Entity does not exist!");

  /* Will be used to later clear the entity registry */
  return gid_vec_push(&entt_item->value.a->entt_delete_queue, &entt);
}

gid gq_create_entity(g_query_t *q) {
  log_debug("conc entt create\n");
  archetype *arch = q->archetype_context;

  /* Entity creations move to cache world */
  gid temp_entt_id = g_create_entity(arch->cache);
  gid_vec_push(&arch->entt_creation_queue, &temp_entt_id);

  return temp_entt_id;
}

retcode gq_queue_delete(g_query_t *q, gid entt) {
  log_debug("conc entt queued to delete: {id: %ld, ctx: %d}", entt,
            SELECT_MODE(entt));
  /* If the entt id belongs to the cache, modify the cache context */
  if (SELECT_MODE(entt) == CACHED) {
    archetype *arch = q->archetype_context;
    return g_queue_delete(arch->cache, entt);
  }

  if (SELECT_MODE(entt) == STORAGE) {
    gid_vec_push(&q->archetype_context->entt_delete_queue, &entt);
  }
  return R_FAIL;
}

retcode __gq_add(g_query_t *q, gid entt, char *name) {
  log_debug("conc add: {id: %ld, name: %s}\n", entt, name);
  /* If entt id belongs to the cache, modify the cache context */
  if (SELECT_MODE(entt) == CACHED)
    return g_add_component(q->archetype_context->cache, entt, name);

  /* Create or access its corresponding cache id and add */
  gid cache_id = _g_access_cache_entt(q->archetype_context, entt);
  return g_add_component(q->archetype_context->cache, cache_id, name);
}

void *__gq_get(g_query_t *q, gid entt, char *name) {
  log_debug("conc get: {id: %ld, name: %s}\n", entt, name);
  if (SELECT_MODE(entt) == CACHED)
    return g_get_component(q->archetype_context->cache, entt, name);

  gid type_name = (gid)hash_bytes(name, strlen(name));
  if (gid_vec_has(&q->archetype_context->type_set, &type_name)) {
    return g_get_component(q->world, entt, name);
  }

  gid        cache_entt_id = _g_access_cache_entt(q->archetype_context, entt);
  archetype *cache_archetype =
      gid_entity_record_map_find(&q->archetype_context->cache->entity_registry,
                                 &cache_entt_id)
          ->value.a;

  if (gid_vec_has(&cache_archetype->type_set, &type_name)) {
    return g_get_component(q->archetype_context->cache, cache_entt_id, name);
  }

  assert("Expected component to exist when deleting. but component DNE!");
  return NULL;
}

retcode __gq_set(g_query_t *q, gid entt, char *name, void *comp) {
  log_debug("conc set: {id: %ld, name: %s}", entt, name);
  /* Giving a cache ID means that the entity has nothing in the actual world.
     Skip to editing the cache. */
  if (SELECT_MODE(entt) == CACHED)
    return g_set_component(q->archetype_context->cache, entt, name, comp);

  /* We *prefer* to always edit the actual composit and retain the
     vectorized property. So we always return a refernce to storage
     whenever possible. */
  gid type_name = (gid)hash_bytes(name, strlen(name));
  if (gid_vec_has(&q->archetype_context->type_set, &type_name)) {
    g_set_component(q->world, entt, name, comp);
    return R_OKAY;
  }

  gid        cache_entt_id = _g_access_cache_entt(q->archetype_context, entt);
  archetype *cache_archetype =
      gid_entity_record_map_find(&q->archetype_context->cache->entity_registry,
                                 &cache_entt_id)
          ->value.a;

  if (gid_vec_has(&cache_archetype->type_set, &type_name)) {
    g_set_component(q->archetype_context->cache, cache_entt_id, name, comp);
    return R_OKAY;
  }

  assert("Expected component to exist when deleting. but component DNE!");
  return R_FAIL;
}

retcode __gq_rem(g_query_t *q, gid entt, char *name) {
  log_debug("conc rem: {id: %ld, name: %s}\n", entt, name);
  /* Very tricky. Component removal makes a mess of things but nothing that
     cannot be recovered from in the migration step.

     There are two cases: this is our first time deleting or not.

     First time case: For the first time, we transition ALL components EXCEPT
     for the one specified for deletion into the cache world. This is
     equivalent for deletion. On the get/set steps, we always prefer the
     components inside of storage over cache is their types intersect.

     Nth time case: For other times the entity gets their component deleted
     in storage, we only now need to transition the archetypes once. */
  if (SELECT_MODE(entt) == CACHED)
    return g_rem_component(q->archetype_context->cache, entt, name);

  gid entt_cache_id = _g_access_cache_entt(q->archetype_context, entt);

  archetype *cache_archetype =
      gid_entity_record_map_find(&q->archetype_context->cache->entity_registry,
                                 &entt_cache_id)
          ->value.a;

  /* We check if this is the first time transitioning by seeing if types
     intersect. If they do then this is not the first time deleting. */
  if (vectors_intersect(&q->archetype_context->type_set,
                        &cache_archetype->type_set))
    return g_rem_component(q->archetype_context->cache, entt_cache_id, name);

  /* Transition into cache only. NOTE: we take the union because we want to
     retain whatever types are already in there */

  gid_vec_t type_set;
  type_union(&q->archetype_context->type_set, &cache_archetype->type_set,
             &type_set);

  _g_transfer_archetypes(q->archetype_context->cache, entt_cache_id, &type_set);

  gid_vec_free(&type_set);
  return g_rem_component(q->archetype_context->cache, entt_cache_id, name);
}

g_itr __gq_vectorize(g_query_t *q) {
  g_itr itr = {0};
  itr.offsets = &q->archetype_context->offsets;
  itr.stored_components = &q->archetype_context->composite;
  return itr;
}

void *__gq_field(g_itr *itr, char *type, gint idx) {
  void *frag = fragment_vec_at(itr->stored_components, idx);

  gid                 type_id = (gid)hash_bytes(type, strlen(type));
  gid_gsize_map_item *item = gid_gsize_map_find(itr->offsets, &type_id);

  assert(item && "field does not have this component!");

  buff_t buff;
  buff_init(&buff, frag);
  return buff_skip(&buff, item->value);
}

fragment *__gq_take(g_query_t *q, fragment *frag, char *name) {
  gid id = (gid)hash_bytes(name, strlen(name));

  gid_gsize_map_item *offset_item =
      gid_gsize_map_find(&q->archetype_context->offsets, &id);

  assert(offset_item && "This component does not belong to this entity!");

  return frag + offset_item->value;
}

fragment *__gq_index(g_query_t *q, gint index, char *name) {
  gid                 id = (gid)hash_bytes(name, strlen(name));
  gid_gsize_map_item *offset_item =
      gid_gsize_map_find(&q->archetype_context->offsets, &id);

  assert(offset_item && "This component does not belong to this entity!");
  return fragment_vec_at(q->component_store, index) + offset_item->value;
}

static gid hash_vec(any_vec_t *v) {
  return (gid)hash_bytes(v->element_head, v->length * v->__el_size);
}

static bool    asc_gids(gid *a, gid *b, void *arg) { return *a < *b; }
static retcode _g_transfer_archetypes(g_core_t *w, gid entt,
                                      gid_vec_t *type_set) {
  archetype *a_next, *a_prev;

  /* By sorting the vector before processing, we effectively turn it into a
     element set. */
  gid_vec_sort(type_set, asc_gids, NULL);
  gid arch_id = hash_vec((any_vec_t *)type_set);

  /* Check archetype_registry to see if this one exists, if not: make it. */
  if (!gid_archetype_map_has(&w->archetype_registry, &arch_id)) {
    archetype a = {0};
    _g_init_archetype(w, &a, type_set);
    gid_archetype_map_put(&w->archetype_registry, &arch_id, &a);

    /* Whenever a new archetype is added to the graph, we must re-schedule
       where each system touches. Therefore, we want to trigger a reprocesses.*/
    w->reprocess_fsm = 1;
  }

  /* Query and load the archetype */
  gid_entity_record_map_item *item =
      gid_entity_record_map_find(&w->entity_registry, &entt);
  entity_record entt_rec = item->value;

  gint index_prev = entt_rec.index;

  a_next = &gid_archetype_map_find(&w->archetype_registry, &arch_id)->value;
  a_prev = entt_rec.a;

  /* Special Case: If its the empty archetype, we can skip the following
     routine and just transfer */
  if (a_prev == &empty_archetype) {

    item->value.a = a_next;
    item->value.index = a_next->composite.length;
    fragment_vec_resize(&a_next->composite, a_next->composite.length + 1);
    entity_record *rec = &item->value;
    gid_pentity_record_map_put(&a_next->entt_members, &entt, &rec);

    return R_OKAY;
  }

  /* We need to maintain the component data we care about in the segment and
     discard the rest. We do this by collecting the intersection between the new
     and old archetypes. */
  gint  new_pos = a_next->composite.length;
  void *seg_prev = fragment_vec_at(&a_prev->composite, entt_rec.index);
  fragment_vec_resize(&a_next->composite, a_next->composite.length + 1);
  void *seg_next = fragment_vec_at(&a_next->composite, new_pos);

  /* Update position of entity */
  item->value.a = a_next;
  item->value.index = new_pos;

  /* Cache update step */
  entity_record *rec = &item->value;
  gid_pentity_record_map_remove(&a_prev->entt_members, &entt);
  gid_pentity_record_map_put(&a_next->entt_members, &entt, &rec);

  // TODO: performance bottleneck, fix in future with sets
  for (size_t i = 0; i < a_prev->type_set.length; i++) {
    gid *type = gid_vec_at(&a_prev->type_set, i);
    if (!gid_vec_has(&a_next->type_set, type)) continue;

    gsize comp_size = gid_gsize_map_find(&w->component_registry, type)->value;
    gsize off_prev = gid_gsize_map_find(&a_prev->offsets, type)->value;
    gsize off_next = gid_gsize_map_find(&a_next->offsets, type)->value;

    memmove(seg_next + off_next, seg_prev + off_prev, comp_size);
  }

  /* Move the component data to the the next archetype. */
  void *seg_overwrite =
      fragment_vec_at(&a_prev->composite, a_prev->composite.length - 1);
  memmove(seg_prev, seg_overwrite, a_prev->composite.__el_size);

  /* Instead of deleting from the composite vector now, we delete at the end
     of the tick as a batch. */
  gint_vec_push(&a_prev->dead_fragments, &index_prev);

  return R_OKAY;
}

static retcode _g_init_archetype(g_core_t *w, archetype *a,
                                 gid_vec_t *type_set) {
  GID_INCR(w->id_gen);
  a->archetype_id = SELECT_ID(w->id_gen);

  /* Setup archetype members. */
  gid_pentity_record_map_init(&a->entt_members);
  psystem_data_vec_init(&a->contenders);
  gid_vec_init(&a->entt_delete_queue);
  gid_vec_init(&a->entt_creation_queue);
  gint_vec_init(&a->dead_fragments);
  gid_gsize_map_init(&a->offsets);
  gid_vec_copy(gid_vec_init(&a->type_set), type_set);

  /* To construct the offset map, we increment and collect the sizes of the
    types in order. */
  gsize curs = 0;
  for (int64_t i = 0; i < a->type_set.length; i++) {
    gid *name_hash = gid_vec_at(&a->type_set, i);
    gid_gsize_map_put(&a->offsets, name_hash, &curs);

    gid_gsize_map_item *component_size =
        gid_gsize_map_find(&w->component_registry, name_hash);
    assert(component_size && "Component was not registered!");
    curs += component_size->value;
  }

  /* The length of 1 element in the composite vector is equal to the final size
     of curs. */
  vec_unknown_type_init((any_vec_t *)&a->composite, curs);

  /* Caches dont get caches! */
  if (SELECT_MODE(w->id_gen) == CACHED) return R_OKAY;
  /* Setup caching context members */
  a->cache = g_create_world();
  a->cache->is_main = 0;
  GID_SET_MODE(a->cache->id_gen, CACHED);

  /* If the first thing done in the cache is a translation, it will fail
     because the 0th id represents empty. The real storage does not have
     this problem because it's impossible to add a component to a non-existent
     entity. */
  GID_INCR(a->cache->id_gen);
  gid_gsize_map_free(&a->cache->component_registry);
  gid_gsize_map_copy(&a->cache->component_registry, &w->component_registry);
  gid_gid_map_init(&a->cache_interface);

  return R_OKAY;
}

typedef struct f_consolidate_entity_args f_consolidate_entity_args;
struct f_consolidate_entity_args {
  g_core_t *world;
  g_core_t *cache;
};
static bool consolidate_entity(gid_gid_map_item *item, void *arg) {
  struct f_consolidate_entity_args *args =
      (struct f_consolidate_entity_args *)arg;

  gid perm_id = item->key;
  gid temp_id = item->value;

  g_core_t *world = args->world;
  g_core_t *cache = args->cache;

  entity_record *perm_rec =
      &gid_entity_record_map_find(&world->entity_registry, &perm_id)->value;

  entity_record *cache_rec =
      &gid_entity_record_map_find(&cache->entity_registry, &temp_id)->value;

  /* Transition all to top level */
  gid_vec_t *old_type_set = &perm_rec->a->type_set;

  _g_transfer_archetypes(world, perm_id, &cache_rec->a->type_set);
  // type_union(&perm_rec->a->type_set, &cache_rec->a->type_set, &type_set);
  // _g_transfer_archetypes(world, perm_id, &type_set);

  /* Look for new components and add */
  gid_vec_t type_set;
  gid_vec_init(&type_set);
  for (size_t type_i = 0; type_i < cache_rec->a->type_set.length; type_i++) {
    gid *temp_type = gid_vec_at(&cache_rec->a->type_set, type_i);

    /* Non cached storage components are preferred. */
    if (gid_vec_has(old_type_set, temp_type)) continue;

    gid_vec_push(&type_set, temp_type);

    __g_set_component(world, perm_id, *temp_type,
                      __g_get_component(cache, temp_id, *temp_type));
    gid_vec_pop(&type_set);
  }
  gid_vec_free(&type_set);
  return true;
}

static bool consolidate_archetype(gid_archetype_map_item *item, void *arg) {
  g_core_t *w = (g_core_t *)arg;
  /* Deletion algorithm:
       - For each entity, in the delete_queue
       - Pop
       - Delete from composite
       - Delete cache link to stop further steps on entity
       - Delete inside cache the ID so we don't accidentally consolidate that
         ID */
  archetype *a = &item->value;

  while (a->entt_delete_queue.length) {
    gid entt = *gid_vec_top(&a->entt_delete_queue);
    gid_vec_pop(&a->entt_delete_queue);

    gid_entity_record_map_remove(&w->entity_registry, &entt);
    gid_pentity_record_map_remove(&a->entt_members, &entt);

    gid_entity_record_map_remove(&a->cache->entity_registry, &entt);
    if (gid_gid_map_has(&a->cache_interface, &entt)) {
      gid cache_entt = gid_gid_map_find(&a->cache_interface, &entt)->value;
      gid_entity_record_map_remove(&a->cache->entity_registry, &cache_entt);
      gid_gid_map_remove(&a->cache_interface, &entt);
    }
  }

  /* Creation algorithm:
       - For each entity, in the creation queue
       - Pop
       - Create a new id
       - Load the cached entity
       - Delete inside cache the ID so we don't accidentally consolidate that
         ID
       - Transition to correct archetype, copy into composite */
  while (a->entt_creation_queue.length) {
    gid cached_entt = *gid_vec_top(&a->entt_creation_queue);
    gid_vec_pop(&a->entt_creation_queue);

    gid entt = g_create_entity(w);

    entity_record *cached_entt_rec =
        &gid_entity_record_map_find(&item->value.cache->entity_registry,
                                    &cached_entt)
             ->value;

    /* Perform archetype transition. */
    __g_add_component(w, entt, &cached_entt_rec->a->type_set);

    /* Perform component data transition */
    // TODO: this can be optimized by directly doing a memmove from cache
    for (gint type_i = 0; type_i < cached_entt_rec->a->type_set.length;
         type_i++) {

      gid  *type = gid_vec_at(&cached_entt_rec->a->type_set, type_i);
      void *seg = __g_get_component(w, cached_entt, *type);
      __g_set_component(w, entt, *type, seg);
    }

    gid_entity_record_map_remove(&a->cache->entity_registry, &cached_entt);
  }

  /* Consolidation algorithm:
     We know that all data currently on the archetype is fresh, so we only
     want to copy the NEW types. We also know that the ONLY remaining entities
     in the cache are all modifications so we can just easily iterate and
     delete */

  gid_gid_map_foreach(
      &a->cache_interface, consolidate_entity,
      &(struct f_consolidate_entity_args){.world = w, .cache = a->cache});

  return true;
}
static void consolidate_fsm(g_core_t *w) {
  gid_archetype_map_foreach(&w->archetype_registry, consolidate_archetype, w);
}

static retcode _g_migrate(g_core_t *g) {
  /* migration functions go here */
  consolidate_fsm(g);
  return R_OKAY;
}

static bool reset_archetype(gid_archetype_map_item *item, void *arg) {
  archetype *cache_arch = &item->value;

  fragment_vec_clear(&cache_arch->composite);
  gid_pentity_record_map_clear(&cache_arch->entt_members);

  return true;
}
static bool clean_archetype_cache(gid_archetype_map_item *item, void *arg) {
  archetype *arch = &item->value;

  gid_vec_clear(&arch->entt_delete_queue);
  gid_vec_clear(&arch->entt_creation_queue);
  gid_gid_map_clear(&arch->cache_interface);

  gid_entity_record_map_clear(&arch->cache->entity_registry);

  gid_archetype_map_foreach(&arch->cache->archetype_registry, reset_archetype,
                            NULL);
  return true;
}
static retcode _g_cleanup(g_core_t *g) {
  gid_archetype_map_foreach(&g->archetype_registry, clean_archetype_cache,
                            NULL);
  return R_OKAY;
}

static gid _g_create_cache_interface(archetype *a, gid entt) {
  /* Create cache item and put in cached item to interface */
  gid cache_id = g_create_entity(a->cache);
  gid_gid_map_put(&a->cache_interface, &entt, &cache_id);
  return cache_id;
}

static gid _g_access_cache_entt(archetype *a, gid entt) {
  gid_pentity_record_map_item *entt_item =
      gid_pentity_record_map_find(&a->entt_members, &entt);
  assert(entt_item && "Entity DNE!");

  archetype *entt_arch = entt_item->value->a;

  gid_gid_map_item *cache_item =
      gid_gid_map_find(&entt_arch->cache_interface, &entt);

  if (cache_item) return cache_item->value;
  return _g_create_cache_interface(a, entt);
}
// gid bsearch_proximity(gint_vec_t *array, gid target) {
//   gint left = 0;
//   gint right = array->length - 1;
//   gid  result = -1;
//   while (left <= right) {
//     gint mid = left + (right - left) / 2;
//     if (*gint_vec_at(array, mid) <= target) {
//       result = mid;
//       left = mid + 1;
//     } else {
//       right = mid - 1;
//     }
//   }
//   return result;
// }

static bool defrag_entity(gid_pentity_record_map_item *item, void *arg) {
  gint_vec_t *rolling_offsets = (gint_vec_t *)arg;
  if (rolling_offsets->length == 0) return true;

  item->value->index -= *gint_vec_at(rolling_offsets, item->value->index);
  return true;
}

static bool defrag_archetype(gid_archetype_map_item *item, void *arg) {
  archetype *a = &item->value;

  fragment_vec_t vectorized;
  vec_unknown_type_init((any_vec_t *)&vectorized, a->composite.__el_size);
  fragment_vec_resize(&vectorized,
                      a->composite.length - a->dead_fragments.length);

  gint_vec_t rolling_offsets;
  gint_vec_init(&rolling_offsets);
  gint dead_index = 0;
  for (gint i = 0; i < a->composite.length; i++) {
    if (dead_index < a->dead_fragments.length &&
        *gint_vec_at(&a->dead_fragments, dead_index) == i) {
      dead_index++;
      gint_vec_push(&rolling_offsets, &dead_index);
      continue;
    }
    void *segment = fragment_vec_at(&a->composite, i);
    fragment_vec_push(&vectorized, segment);
    gint_vec_push(&rolling_offsets, &dead_index);
  }

  fragment_vec_free(&a->composite);
  memmove(&a->composite, &vectorized, sizeof(fragment_vec_t));

  gid_pentity_record_map_foreach(&a->entt_members, defrag_entity,
                                 &rolling_offsets);

  gint_vec_free(&rolling_offsets);
  return true;
}
static void _g_defragment(g_core_t *w) {
  gid_archetype_map_foreach(&w->archetype_registry, defrag_archetype, w);
}