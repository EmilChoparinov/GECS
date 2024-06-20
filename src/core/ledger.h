#ifndef __HEADER_LEDGER_H__
#define __HEADER_LEDGER_H__

#include "register.h"

typedef struct archetype_cache archetype_cache;
struct archetype_cache {
  any_vec_t composite;    /* Interleaved components of this cache. */
  gid       archetype_id; /* The associated archetype id with the cache */

  /* Pointer to the offsets stored in the real archetype. This is ok to do
   * because offsets is effectively read-only  */
  gid_gsize_map_t *offsets;
};
VECTOR_GEN_H(archetype_cache);

typedef struct entity_cache entity_cache;
struct entity_cache {
  archetype_cache *a;     /* Cache thie entity belongs to */
  gint             index; /* The index to collect their components */
};

MAP_GEN_H(gid, entity_cache);

typedef struct ledger ledger;
struct ledger {
  archetype_cache_vec_t  archetype_mutations; /* Vec : archetype cache */
  gid_entity_cache_map_t entt_cache;          /* Map : gid -> entt cache */
};

#endif