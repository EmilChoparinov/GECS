#include "str_utils.h"

uint64_t hash_vector(vec *v) {
  return hash_bytes(v->elements, v->length * v->__el_size);
}

feach(push_to_set, uint64_t, hash, {
  set *types = (set *)args;
  set_put(types, &hash);
});
set *vec_to_set(vec *v, set *out) {
  __set_init(out, v->__el_size, get_frame_ctx(), TO_HEAP, v->length);
  vec_foreach(v, push_to_set, out);
  return out;
}
vec *set_to_vec(set *s, vec *out) { return map_to_vec(&s->internals, out); }

bool set_is_subset(set *super, set *maybe_sub) {
  vec to_vec;
  set_to_vec(maybe_sub, &to_vec);

  for (int64_t i = 0; i < to_vec.length; i++) {
    kvpair kv = read_kvpair(&maybe_sub->internals, vec_at(&to_vec, i));
    if (!set_has(super, kv.key)) {
      return false;
    }
  }

  return true;
}