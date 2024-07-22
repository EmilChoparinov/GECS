#include "types.h"

VEC_TYPE_IMPL(composite, void *);

VEC_TYPE_IMPL(id_vec, gid);
VEC_TYPE_IMPL(int64_vec, int64_t);
VEC_TYPE_IMPL(system_vec, system_data);

MAP_TYPE_IMPL(id_to_size, gid, gsize);
MAP_TYPE_IMPL(id_to_id, gid, gid);
MAP_TYPE_IMPL(id_to_int64, gid, int64_t);
MAP_TYPE_IMPL(id_to_archetype, gid, archetype);

MAP_TYPE_IMPL(id_to_hash, gid, uint64_t);
MAP_TYPE_IMPL(hash_to_size, uint64_t, gsize);
VEC_TYPE_IMPL(hash_vec, uint64_t);
MAP_TYPE_IMPL(hash_to_archetype, uint64_t, archetype);

SET_TYPE_IMPL(type_set, int64_t);

/* Cache generated types. These vectors contain only address to other data
   found in the structures above or just addresses in general. */
VEC_TYPE_IMPL(cache_vec, void *);
MAP_TYPE_IMPL(cache_map, gid, void *);
