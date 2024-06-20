#include "gecs.h"

typedef struct system_data system_data;
typedef system_data       *psystem_data;
struct system_data {
  g_system *run_system;   /* A function pointer to the system. */
  gid_vec_t requirements; /* String of components in the form "A,B,C". */
};
VECTOR_GEN_H(system_data);
VECTOR_GEN_C(system_data);

VECTOR_GEN_H(psystem_data);
VECTOR_GEN_C(psystem_data);

struct archetype {
  gid archetype_id; /* Unique Identifier for this archetype. */

  gid_vec_t          type_set;   /* Ordered Vec: hash(comp names) */
  any_vec_t          composite;  /* Contiguous vector of interleaved compents */
  psystem_data_vec_t contenders; /* Vec: system_data* */

  gid_gsize_map_t offsets; /* Map: hash(name) -> interleaved comp offset */

  /* The following members are made for caching purposes. */
  ledger cache;        /* In concurrency contexts, mutations are stored here. */
  gid    tailing_entt; /* Used to optimize defragmentation */
};
MAP_GEN_H(gid, archetype);
MAP_GEN_C(gid, archetype);

typedef struct entity_record entity_record;
struct entity_record {
  archetype *a;     /* The archetype this entity belongs to. */
  gint       index; /* The index to collect their components. */
};

MAP_GEN_H(gid, entity_record);
MAP_GEN_C(gid, entity_record);

struct g_core_t {
  gid    id_gen;        /* Counts up, makes unique IDs. */
  int8_t reprocess_fsm; /* When this flag is set to 1. It will reprocess the
                           FSM before next tick. */

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
static void    _g_free_archetype(archetype *a);
static retcode _g_transfer_archetypes(g_core_t *w, gid entt, char *query);
static retcode _g_archetype_key(char *types, gid_vec_t *hashes);
static retcode _g_assign_fsm(g_core_t *w);
static gid     hash_vec(any_vec_t *v);

/*-------------------------------------------------------
 * Globals
 *-------------------------------------------------------*/
/* We define the empty archetype to be an arbitrary unique address.*/
static archetype empty_archetype;

g_core_t *g_create_world(void) {
  g_core_t *w = malloc(sizeof(g_core_t));

  w->id_gen = 0;
  w->reprocess_fsm = 1;

  gid_gsize_map_init(&w->component_registry);
  gid_entity_record_map_init(&w->entity_registry);
  gid_archetype_map_init(&w->archetype_registry);

  system_data_vec_init(&w->system_registry);

  /* Curiously, I decided to not put the empty archetype into the registry.
     Why? I don't want to risk adding an extra collision when querying. This if
     ok because the empty archetype represents garbage so a user will never
     query for it. */

  return w;
}

static bool f_free_archetype(gid_archetype_map_item *it) {
  _g_free_archetype(&it->value);
  return true;
}
retcode g_destroy_world(g_core_t *w) {
  gid_gsize_map_free(&w->component_registry);
  gid_entity_record_map_free(&w->entity_registry);
  gid_archetype_map_foreach(&w->archetype_registry, f_free_archetype);

  system_data_vec_free(&w->system_registry);

  return R_OKAY;
}

retcode g_progress(g_core_t *w) {
  if (w->reprocess_fsm) _g_assign_fsm(w);
  return R_OKAY;
}

gid g_create_entity(g_core_t *w) {
  gid id = w->id_gen++;

  /* All entities start initially at the empty archetype */
  gid_entity_record_map_put(
      &w->entity_registry, &id,
      &(entity_record){.a = &empty_archetype, .index = 0});

  return id;
}

retcode g_register_component(g_core_t *w, char *name, size_t component_size) {
  gid hash_name = (gid)hash_bytes(name, strlen(name));
  return gid_gsize_map_put(&w->component_registry, &hash_name, &component_size);
}

retcode g_register_system(g_core_t *w, g_system sys, char *query) {
  assert(query);
  assert(sys);

  gid_vec_t type_set;
  _g_archetype_key(query, &type_set);

  return system_data_vec_push(
      &w->system_registry,
      &(system_data){.requirements = type_set, .run_system = &sys});
}

retcode g_add_component(g_core_t *w, gid entt_id, char *name,
                        size_t comp_size) {
  return _g_transfer_archetypes(w, entt_id, name);
}

void *g_get_component(g_core_t *w, gid entt_id, char *name) {
  gid_entity_record_map_item *item =
      gid_entity_record_map_find(&w->entity_registry, &entt_id);
  assert(item && "Entity does not exist!");

  entity_record entt_rec = item->value;
  gid           type = (gid)hash_bytes(name, strlen(name));

  gsize comp_off = gid_gsize_map_find(&entt_rec.a->offsets, &type)->value;
  return any_vec_at(&entt_rec.a->composite, entt_rec.index) + comp_off;
}

static retcode _g_assign_fsm(g_core_t *w) {
  for (size_t sys_i = 0; sys_i < w->system_registry.length; sys_i++) {
    // system_data *sd = system_data_vec_at(&w->system_registry, sys_i);

    /* For each system, check each archetype and distribute. */
    // gid_archetype_map_item_vec_at
    for (size_t arc_i = 0; arc_i < w->archetype_registry.map.length; arc_i++) {
    }
  }
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

static gid hash_vec(any_vec_t *v) {
  return (gid)hash_bytes(v->element_head, v->length * v->__el_size);
}

static retcode _g_transfer_archetypes(g_core_t *w, gid entt, char *query) {
  archetype *a_next, *a_prev;

  /* Construct the type set and the actual type hash id */
  gid_vec_t type_set;
  _g_archetype_key(query, &type_set);
  gid arch_id = hash_vec((any_vec_t *)&type_set);

  /* Check archetype_registry to see if this one exists, if not: make it. */
  if (!gid_archetype_map_has(&w->archetype_registry, &arch_id)) {
    archetype a;
    _g_init_archetype(w, &a, &type_set);
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
    any_vec_resize(&a_next->composite, a_next->composite.length + 1);
    a_next->tailing_entt = entt;

    return R_OKAY;
  }

  /* We need to maintain the component data we care about in the segment and
     discard the rest. We do this by collecting the intersection between the new
     and old archetypes. */
  void *seg_prev = any_vec_at(&a_prev->composite, entt_rec.index);

  item->value.a = a_next;
  item->value.index = a_next->composite.length;
  any_vec_resize(&a_next->composite, a_next->composite.length + 1);
  a_next->tailing_entt = entt;

  void *seg_next = any_vec_at(&a_next->composite, entt_rec.index);

  gid_vec_t retained_types;
  gid_vec_init(&retained_types);

  // TODO: performance bottleneck, fix in future
  for (size_t i = 0; i < a_prev->type_set.length; i++) {
    gid *type = gid_vec_at(&a_prev->type_set, i);
    if (!gid_vec_has(&a_next->type_set, type)) continue;

    gsize comp_size = gid_gsize_map_find(&w->component_registry, type)->value;
    gsize off_prev = gid_gsize_map_find(&a_prev->offsets, type)->value;
    gsize off_next = gid_gsize_map_find(&a_next->offsets, type)->value;

    memmove(seg_next + off_next, seg_prev + off_prev, comp_size);
  }

  /* We need to defragment the previous archetype. We do this by swapping the
     last element with the new hole and updating the associated entity id. */
  entity_record tail_rec =
      gid_entity_record_map_find(&w->entity_registry, &a_prev->tailing_entt)
          ->value;

  void *seg_overwrite =
      any_vec_at(&a_prev->composite, a_prev->composite.length - 1);
  memmove(seg_prev, seg_overwrite, a_prev->composite.__el_size);

  tail_rec.index = index_prev;
  any_vec_resize(&a_prev->composite, a_prev->composite.length - 1);
  return R_OKAY;
}

static retcode _g_init_archetype(g_core_t *w, archetype *a,
                                 gid_vec_t *type_set) {
  a->archetype_id = w->id_gen++;
  a->tailing_entt = 0;

  psystem_data_vec_init(&a->contenders);
  gid_gsize_map_init(&a->offsets);

  /* Store it locally */
  gid_vec_copy(gid_vec_init(&a->type_set), type_set);

  /* To construct the offset map, we increment and collect the sizes of the
    types in order. */
  gsize curs = 0;
  for (int64_t i = 0; i < a->type_set.length; i++) {
    gid *name_hash = gid_vec_at(&a->type_set, i);
    gid_gsize_map_put(&a->offsets, name_hash, &curs);

    gid_gsize_map_item component_size =
        *gid_gsize_map_find(&w->component_registry, name_hash);
    curs += component_size.value;
  }

  /* The length of 1 element in the composite vector is equal to the final size
     of curs. */
  vec_unknown_type_init(&a->composite, curs);

  // TODO: add ledger
  return R_OKAY;
}
static void _g_free_archetype(archetype *a) {
  gid_vec_free(&a->type_set);
  vec_unknown_type_free(&a->composite);
  psystem_data_vec_free(&a->contenders);

  gid_gsize_map_free(&a->offsets);

  // TODO: add ledger
}