#include "str_utils.h"

uint64_t hash_vector(vec *v) {
  return hash_bytes(v->elements, v->length * v->__el_size);
}

feach(push_to_set, uint64_t, hash, {
  set *types = (set *)args;
  set_put(types, &hash);
});
set *vec_to_set(vec *v, set *out) {
  /* This is supposed to go to stack, just like set_to_vec but I am
     lazy. This function ended up only being used in archetype.c in a
     persistant context. */
  __set_init(out, v->__el_size, get_frame_ctx(), TO_HEAP, v->length);
  vec_foreach(v, push_to_set, out);
  return out;
}
vec *set_to_vec(set *s, vec *out) {
  int32_t old_flags = s->internals.flags;
  s->internals.flags = TO_STACK;
  map_to_vec(&s->internals, out);
  s->internals.flags = old_flags;
  return out;
}

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