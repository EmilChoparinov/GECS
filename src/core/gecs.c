#include "gecs.h"

typedef struct system_data system_data;
typedef system_data       *psystem_data;
struct system_data {
  g_system *run_system;   /* A function pointer to the system. */
  char     *requirements; /* String of components in the form "A,B,C". */
};
VECTOR_GEN_H(system_data);
VECTOR_GEN_C(system_data);

VECTOR_GEN_H(psystem_data);
VECTOR_GEN_C(psystem_data);

typedef struct archetype archetype;
typedef archetype       *parchetype;
struct archetype {
  gid archetype_id; /* Unique Identifier for this archetype. */

  any_vec_t       components; /* Contiguous vector of interleaved compents */
  gid_gsize_map_t offsets;    /* Map: hash(name) -> interleaved comp offset */

  psystem_data_vec_t contenders; /* Vec: system_data* */
};
MAP_GEN_H(gid, archetype);
MAP_GEN_C(gid, archetype);

MAP_GEN_H(gid, parchetype);
MAP_GEN_C(gid, parchetype);

struct g_core_t {
  gid    id_gen;        /* Counts up, makes unique IDs. */
  int8_t reprocess_fsm; /* When this flag is set to 1. It will reprocess the
                           FSM before next tick. */

  /* These are all gid maps because the hashing function outputs the same size
     as gid, so it's convenient. */
  gid_gsize_map_t      component_registry; /* Map: hash(name) -> comp size */
  gid_parchetype_map_t entity_registry;    /* Map: entity id -> archetype* */
  gid_archetype_map_t  archetype_registry; /* Map: hash([name]) -> archetype */

  system_data_vec_t system_registry; /* Vec: system_data. */
};

/*-------------------------------------------------------
 * Static Forward Declarations
 *-------------------------------------------------------*/
static retcode _g_init_archetype(g_core_t *w, archetype *a);
static gid     _g_archetype_key(char *component_query);

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
  gid_parchetype_map_init(&w->entity_registry);
  gid_archetype_map_init(&w->archetype_registry);

  system_data_vec_init(&w->system_registry);

  /* Curiously, I decided to not put the empty archetype into the registry.
     Why? I don't want to risk adding an extra collision when querying. This if
     ok because the empty archetype represents garbage so a user will never
     query for it. */

  return w;
}

static bool f_free_archetype(gid_archetype_map_item *it) {
  archetype a = it->value;
  vec_unknown_type_free(&a.components);
  psystem_data_vec_free(&a.contenders);
  gid_gsize_map_free(&a.offsets);
  return true;
}
retcode g_destroy_world(g_core_t *w) {
  gid_gsize_map_free(&w->component_registry);
  gid_parchetype_map_free(&w->entity_registry);
  gid_archetype_map_foreach(&w->archetype_registry, f_free_archetype);

  system_data_vec_free(&w->system_registry);

  return R_OKAY;
}

retcode g_progress(g_core_t *w);

gid g_create_entity(g_core_t *w) {
  gid id = w->id_gen++;

  /* All entities start initially at the empty archetype */
  gid_parchetype_map_put(&w->entity_registry, &id, &empty_archetype);

  return id;
}

retcode g_register_component(g_core_t *w, char *name, size_t component_size) {
  gid hash_name = (gid)hash_bytes(name, strlen(name));
  return gid_gsize_map_put(&w->component_registry, &hash_name, component_size);
}

/* Static Function Implementations */
static gid _g_archetype_key(char *component_query) {
  assert(component_query);
  /* Since archetypes are sets, and query strings can be in the various forms
     such as "A,B,C" or "B,A,C", we need a mechanism that maps all string
     combinations into a set. We perform a sort operation with a stack vector
     over the hash of each individual component. This allows us to rehash the
     normalized vector and produce a key that works for all iterations of
     "A,B,C".

     Collision Detection:
     Since we are using hashing algorithms here we must be careful with
     collisions. The way we protect against collisions is by caching the
     archetype query string and doing a memory check if they are the same. If
     a hash leads to an archetype that is **not** the archetype queried for,
     the library will assert and exit.
     */

  /* The group size for the regex matcher is the count of ',' in the string.
     Therefore we interate and count how many there are. */
  gint   archetype_len = 0;
  size_t i, str_len = strlen(component_query);
  for (i = 0; i < str_len && i < GECS_MAX_ARCHETYPE_COMPOSITION; i++)
    if (component_query[i] == ',') archetype_len++;
  assert(i != GECS_MAX_ARCHETYPE_COMPOSITION &&
         "Compositions allowed up to 256!");

  gid_vec_t hashes;
  gid_vec_init(&hashes);

  regex_t    matcher;
  regmatch_t groups[GECS_MAX_ARCHETYPE_COMPOSITION];
  regcomp(&matcher, "(\\w+)", REG_EXTENDED);

  /* Assert that the query actually got matches. */
  assert(regexec(&matcher, component_query, GECS_MAX_ARCHETYPE_COMPOSITION,
                 groups, 0) != 0);

  for (i = 1; i < str_len; i++) {
    /* groups[0] contains the full matched string, not groups. */
    regoff_t arg_start = groups[i].rm_so;
    regoff_t arg_end = groups[i].rm_eo;

    gid component_id =
        (gid)hash_bytes(&component_query[arg_start], arg_end - arg_start);

    if (hashes.length == 0) {
      gid_vec_push(&hashes, &component_id);
      continue;
    }

    if (*gid_vec_top(&hashes) < component_id) {
      gid *old_top = gid_vec_top(&hashes);
      gid_vec_pop(&hashes);
      gid_vec_push(&hashes, &component_id);
      gid_vec_push(&hashes, old_top);
      continue;
    }

    gid_vec_push(&hashes, &component_id);
  }

  regfree(&matcher);

  /* To generate the final hash location: */
  return (gid)hash_bytes(hashes.element_head, hashes.length * sizeof(gid));
};