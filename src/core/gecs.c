#include "gecs.h"

typedef struct archetype archetype;
MAP_GEN_FORWARD(gid, archetype);

struct archetype {
  any_vec_t       components; /* Contiguous vector of interleaved components. */
  gstr_gint_map_t offsets;    /* Map: name -> offset of component interleaved */

  gid_archetype_map_t *edges; /* Map: archetype id -> archetype* */
};

MAP_GEN_H(gid, archetype);
MAP_GEN_C(gid, archetype);

// SET_GEN_H(gint);

// /* We make this struct packed because the set will be used as a key. */
// SET_GEN_C(gint, __attribute__((packed))); 
// typedef gint_set_t archetype_t;    /* An archetype is a set of component ids */
// MAP_GEN_H(archetype_t, archetype); /* Map: set<gint> -> archetype data */

struct g_core_t {
  gid    id_gen;        /* Counts up, makes unique IDs. */
  int8_t reprocess_fsm; /* When this flag is set to 1. It will reprocess the
                           FSM before next tick. */

  gstr_gsize_map_t    component_registry; /* Map: name -> component size */
  gid_gid_map_t       entity_registry;    /* Map: entity id -> archetype id */
  gid_archetype_map_t archetype_registry; /* Map: archetype id -> archetype */

  /*-------------------------------------------------------
   * Cache
   *-------------------------------------------------------*/
};
