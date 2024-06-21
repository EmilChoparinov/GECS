#include "register.h"

/*-------------------------------------------------------
 * Vector Types
 *-------------------------------------------------------*/
VECTOR_GEN_C(bool);
VECTOR_GEN_C(gid);

#include "map.h"

/*-------------------------------------------------------
 * Map Types
 *-------------------------------------------------------*/
MAP_GEN_C(gid, gid);
// MAP_GEN_C(gid, gsize);
static void gid_gsize_map_item_assert_init_checks(gid_gsize_map_item_vec_t *v);
static void gid_gsize_map_item_bound_assert_checks(gid_gsize_map_item_vec_t *v,
                                                   int64_t                   i);
static gid_gsize_map_item_vec_t *
gid_gsize_map_item_vec_construct(gid_gsize_map_item_vec_t *v);
static gid_gsize_map_item *
gid_gsize_map_item__d_vec_at(gid_gsize_map_item_vec_t *v, int64_t i);
gid_gsize_map_item_vec_t *
gid_gsize_map_item_vec_construct(gid_gsize_map_item_vec_t *v) {
  v->length = 0;
  v->__size = 1;
  v->__top = 0;
  return v;
}
gid_gsize_map_item_vec_t *
gid_gsize_map_item_vec_init(gid_gsize_map_item_vec_t *v) {
  gid_gsize_map_item_vec_construct(v);
  v->__el_size = sizeof(gid_gsize_map_item);
  v->element_head = calloc(v->__size, v->__el_size);
  ((v->element_head)
       ? (void)(0)
       : __assert_fail("v->element_head", "/home/emil/GECS/src/core/register.c",
                       15, __extension__ __PRETTY_FUNCTION__));
  return v;
}
retcode gid_gsize_map_item_vec_free(gid_gsize_map_item_vec_t *v) {
  ((v) ? (void)(0)
       : __assert_fail("v", "/home/emil/GECS/src/core/register.c", 15,
                       __extension__ __PRETTY_FUNCTION__));
  ((v->element_head != ((void *)0))
       ? (void)(0)
       : __assert_fail("v->element_head != NULL",
                       "/home/emil/GECS/src/core/register.c", 15,
                       __extension__ __PRETTY_FUNCTION__));
  free(v->element_head);
  return R_OKAY;
}
gid_gsize_map_item_vec_t *gid_gsize_map_item_vec_heap_init(void) {
  gid_gsize_map_item_vec_t *v = malloc(sizeof(*v));
  ((v) ? (void)(0)
       : __assert_fail("v", "/home/emil/GECS/src/core/register.c", 15,
                       __extension__ __PRETTY_FUNCTION__));
  gid_gsize_map_item_vec_init(v);
  return v;
}
void gid_gsize_map_item_vec_heap_free(gid_gsize_map_item_vec_t *v) {
  gid_gsize_map_item_vec_free(v);
  free(v);
}
retcode gid_gsize_map_item_vec_resize(gid_gsize_map_item_vec_t *v,
                                      int64_t                   size) {
  gid_gsize_map_item_assert_init_checks(v);
  v->length = size;
  if (size < v->__size) return R_OKAY;
  int64_t old_size = v->__size;
  while (size >= v->__size) v->__size *= 2;
  void *new_addr = calloc(v->__size * v->__el_size, 1);
  ((new_addr) ? (void)(0)
              : __assert_fail("new_addr", "/home/emil/GECS/src/core/register.c",
                              15, __extension__ __PRETTY_FUNCTION__));
  memcpy(new_addr, v->element_head, old_size * v->__el_size);
  free(v->element_head);
  v->element_head = new_addr;
  return R_OKAY;
}
gid_gsize_map_item *gid_gsize_map_item_vec_at(gid_gsize_map_item_vec_t *v,
                                              int64_t                   i) {
  gid_gsize_map_item_bound_assert_checks(v, i);
  return gid_gsize_map_item__d_vec_at(v, i);
}
retcode gid_gsize_map_item_vec_put(gid_gsize_map_item_vec_t *v, int64_t i,
                                   gid_gsize_map_item *element) {
  gid_gsize_map_item_bound_assert_checks(v, i);
  ((element) ? (void)(0)
             : __assert_fail("element", "/home/emil/GECS/src/core/register.c",
                             15, __extension__ __PRETTY_FUNCTION__));
  memmove((char *)v->element_head + i * v->__el_size, element, v->__el_size);
  return R_OKAY;
}
retcode gid_gsize_map_item_vec_delete(gid_gsize_map_item_vec_t *v,
                                      gid_gsize_map_item       *el) {
  gid_gsize_map_item_assert_init_checks(v);
  int64_t idx = gid_gsize_map_item_vec_find(v, el);
  ((idx != -1)
       ? (void)(0)
       : __assert_fail("idx != -1", "/home/emil/GECS/src/core/register.c", 15,
                       __extension__ __PRETTY_FUNCTION__));
  if (idx == v->length - 1) {
    gid_gsize_map_item_vec_pop(v);
    return R_OKAY;
  }
  void *loc = (char *)v->element_head + idx * v->__el_size;
  void *loc_next = (char *)v->element_head + (idx + 1) * v->__el_size;
  memmove(loc, loc_next, v->length * v->__el_size - idx - v->__el_size);
  v->length--;
  return R_OKAY;
}
_Bool gid_gsize_map_item_vec_has(gid_gsize_map_item_vec_t *v,
                                 gid_gsize_map_item       *element) {
  gid_gsize_map_item_assert_init_checks(v);
  ((element) ? (void)(0)
             : __assert_fail("element", "/home/emil/GECS/src/core/register.c",
                             15, __extension__ __PRETTY_FUNCTION__));
  return gid_gsize_map_item_vec_find(v, element) != -1;
}
int64_t gid_gsize_map_item_vec_find(gid_gsize_map_item_vec_t *v,
                                    gid_gsize_map_item       *element) {
  gid_gsize_map_item_assert_init_checks(v);
  ((element) ? (void)(0)
             : __assert_fail("element", "/home/emil/GECS/src/core/register.c",
                             15, __extension__ __PRETTY_FUNCTION__));
  for (int64_t i = 0; i < v->length; i++)
    if (memcmp(element, gid_gsize_map_item_vec_at(v, i), v->__el_size) == 0)
      return i;
  return -1;
}
retcode gid_gsize_map_item_vec_push(gid_gsize_map_item_vec_t *v,
                                    gid_gsize_map_item       *element) {
  gid_gsize_map_item_assert_init_checks(v);
  ((element) ? (void)(0)
             : __assert_fail("element", "/home/emil/GECS/src/core/register.c",
                             15, __extension__ __PRETTY_FUNCTION__));
  _Bool ret = gid_gsize_map_item_vec_resize(v, v->length) == R_OKAY;
  ((ret) ? (void)(0)
         : __assert_fail("ret", "/home/emil/GECS/src/core/register.c", 15,
                         __extension__ __PRETTY_FUNCTION__));
  memmove(gid_gsize_map_item__d_vec_at(v, v->__top), element, v->__el_size);
  v->__top++;
  if (v->__top >= v->length) v->length = v->__top;
  return ret;
}
void gid_gsize_map_item_vec_pop(gid_gsize_map_item_vec_t *v) {
  gid_gsize_map_item_assert_init_checks(v);
  ((v->__top) ? (void)(0)
              : __assert_fail("v->__top", "/home/emil/GECS/src/core/register.c",
                              15, __extension__ __PRETTY_FUNCTION__));
  if (v->__top == v->length) v->length--;
  v->__top--;
}
gid_gsize_map_item *gid_gsize_map_item_vec_top(gid_gsize_map_item_vec_t *v) {
  gid_gsize_map_item_assert_init_checks(v);
  return gid_gsize_map_item_vec_at(v, v->__top - 1);
}
retcode gid_gsize_map_item_vec_copy(gid_gsize_map_item_vec_t *dest,
                                    gid_gsize_map_item_vec_t *src) {
  gid_gsize_map_item_assert_init_checks(dest);
  gid_gsize_map_item_assert_init_checks(src);
  gid_gsize_map_item_vec_free(dest);
  memmove(dest, src, sizeof(*src));
  if (dest->length == 0) return R_OKAY;
  dest->element_head = malloc(src->__size * src->__el_size);
  memmove(dest->element_head, src->element_head, src->__size * src->__el_size);
  return R_OKAY;
}
retcode gid_gsize_map_item_vec_clear(gid_gsize_map_item_vec_t *v) {
  gid_gsize_map_item_assert_init_checks(v);
  v->length = 0;
  v->__top = 0;
  return R_OKAY;
}
int64_t gid_gsize_map_item_vec_count_if(gid_gsize_map_item_vec_t    *v,
                                        gid_gsize_map_item_predicate f_pred) {
  gid_gsize_map_item_assert_init_checks(v);
  int64_t counter = 0;
  for (int64_t i = 0; i < v->length; i++)
    if (f_pred(&v->element_head[i])) counter++;
  return counter;
}
gid_gsize_map_item_vec_t *
gid_gsize_map_item_vec_filter(gid_gsize_map_item_vec_t    *v,
                              gid_gsize_map_item_predicate f_pred) {
  gid_gsize_map_item_assert_init_checks(v);
  gid_gsize_map_item_vec_t filter;
  gid_gsize_map_item_vec_init(&filter);
  for (int64_t i = 0; i < v->length; i++)
    if (f_pred(&v->element_head[i]))
      gid_gsize_map_item_vec_push(&filter, &v->element_head[i]);
  gid_gsize_map_item_vec_free(v);
  memmove(v, &filter, sizeof(*v));
  return v;
}
void gid_gsize_map_item_vec_foreach(gid_gsize_map_item_vec_t    *v,
                                    gid_gsize_map_item_predicate f_pred) {
  gid_gsize_map_item_assert_init_checks(v);
  for (int64_t i = 0; i < v->length; i++) f_pred(&v->element_head[i]);
}
gid_gsize_map_item_vec_t *
gid_gsize_map_item_vec_map(gid_gsize_map_item_vec_t *v,
                           gid_gsize_map_item_unary  f_unary) {
  gid_gsize_map_item_assert_init_checks(v);
  for (int64_t i = 0; i < v->length; i++)
    v->element_head[i] = f_unary(v->element_head[i]);
  return v;
}
gid_gsize_map_item
gid_gsize_map_item_vec_foldl(gid_gsize_map_item_vec_t *v,
                             gid_gsize_map_item_binary f_binary,
                             gid_gsize_map_item        start) {
  gid_gsize_map_item_assert_init_checks(v);
  gid_gsize_map_item result = start;
  for (int64_t i = 0; i < v->length; i++)
    result = f_binary(result, &v->element_head[i]);
  return result;
}
static void gid_gsize_map_item_assert_init_checks(gid_gsize_map_item_vec_t *v) {
  ((v) ? (void)(0)
       : __assert_fail("v", "/home/emil/GECS/src/core/register.c", 15,
                       __extension__ __PRETTY_FUNCTION__));
  ((v->element_head)
       ? (void)(0)
       : __assert_fail("v->element_head", "/home/emil/GECS/src/core/register.c",
                       15, __extension__ __PRETTY_FUNCTION__));
}
static void gid_gsize_map_item_bound_assert_checks(gid_gsize_map_item_vec_t *v,
                                                   int64_t i) {
  gid_gsize_map_item_assert_init_checks(v);
  ((i >= 0 && i < v->length)
       ? (void)(0)
       : __assert_fail("i >= 0 && i < v->length",
                       "/home/emil/GECS/src/core/register.c", 15,
                       __extension__ __PRETTY_FUNCTION__));
}
static gid_gsize_map_item *
gid_gsize_map_item__d_vec_at(gid_gsize_map_item_vec_t *v, int64_t i) {
  return (void *)(v->element_head) + i * v->__el_size;
};
static uint64_t  gid_gsize_hash(gid *m);
static int64_t   gid_gsize_key_loc(gid_gsize_map_t *m, gid *key);
static void      gid_gsize_map_assert_init(gid_gsize_map_t *m);
static void      gid_gsize_map_lf_property(gid_gsize_map_t *m);
gid_gsize_map_t *gid_gsize_map_init(gid_gsize_map_t *m) {
  m->__size = 16;
  m->slots_in_use = 0;
  gid_gsize_map_item_vec_init(&m->map);
  gid_gsize_map_item_vec_resize(&m->map, m->__size);
  m_bool_vec_init(&m->is_idx_open);
  m_bool_vec_resize(&m->is_idx_open, m->__size);
  m_bool_vec_map(&m->is_idx_open, m_bool_set_to_true);
  return m;
}
void gid_gsize_map_free(gid_gsize_map_t *m) {
  gid_gsize_map_assert_init(m);
  gid_gsize_map_item_vec_free(&m->map);
  m_bool_vec_free(&m->is_idx_open);
}
gid_gsize_map_t *gid_gsize_map_heap_init(void) {
  gid_gsize_map_t *m = malloc(sizeof(*m));
  ((m) ? (void)(0)
       : __assert_fail("m", "/home/emil/GECS/src/core/register.c", 15,
                       __extension__ __PRETTY_FUNCTION__));
  gid_gsize_map_init(m);
  return m;
}
void gid_gsize_map_heap_free(gid_gsize_map_t *m) {
  gid_gsize_map_assert_init(m);
  m->__size = 16;
  m->slots_in_use = 0;
  gid_gsize_map_free(m);
  free(m);
}
any_vec_t *gid_gsize_map_to_vec(gid_gsize_map_t *m, any_vec_t *v) {
  return gid_gsize_map_filter(m, v, (gid_gsize_predicate)m_bool_set_to_true);
}
any_vec_t *gid_gsize_map_ptrs(gid_gsize_map_t *m, any_vec_t *v) {
  gid_gsize_map_assert_init(m);
  vec_unknown_type_init(v, sizeof(void *));
  for (int64_t i = 0; i < m->is_idx_open.length; i++) {
    if (!*m_bool_vec_at(&m->is_idx_open, i)) {
      gid_gsize_map_item *item = gid_gsize_map_item_vec_at(&m->map, i);
      void               *p = &item->value;
      any_vec_push(v, &p);
    }
  }
  return v;
}
gid_gsize_map_item *gid_gsize_map_find(gid_gsize_map_t *m, gid *key) {
  gid_gsize_map_assert_init(m);
  int64_t idx = gid_gsize_key_loc(m, key);
  if (idx != -1) return gid_gsize_map_item_vec_at(&m->map, idx);
  return ((void *)0);
}
retcode gid_gsize_map_put(gid_gsize_map_t *m, gid *key, gsize *value) {
  gid_gsize_map_item item = {0};
  memmove(&item.key, key, sizeof(*key));
  memmove(&item.value, value, sizeof(*value));
  gid_gsize_map_assert_init(m);
  gid_gsize_map_lf_property(m);
  uint64_t start_idx, idx;
  _Bool    false_flag = 0;
  start_idx = gid_gsize_hash(key) % m->map.length;
  for (idx = start_idx; idx < (uint64_t)m->map.length; idx++) {
    if (*m_bool_vec_at(&m->is_idx_open, idx)) {
      m_bool_vec_put(&m->is_idx_open, idx, &false_flag);
      m->slots_in_use++;
      gid_gsize_map_item_vec_put(&m->map, idx, &item);
      return R_OKAY;
    }
    if (memcmp(&gid_gsize_map_item_vec_at(&m->map, idx)->key, key,
               sizeof(*key)) == 0)
      return R_FAIL;
  }
  for (idx = 0; idx < start_idx; idx++) {
    if (*m_bool_vec_at(&m->is_idx_open, idx)) {
      m_bool_vec_put(&m->is_idx_open, idx, &false_flag);
      m->slots_in_use++;
      gid_gsize_map_item_vec_put(&m->map, idx, &item);
      return R_OKAY;
    }
    if (memcmp(&gid_gsize_map_item_vec_at(&m->map, idx)->key, key,
               sizeof(*key)) == 0)
      return R_FAIL;
  }
  return R_FAIL;
}
_Bool gid_gsize_map_has(gid_gsize_map_t *m, gid *key) {
  return gid_gsize_map_find(m, key) != ((void *)0);
}
retcode gid_gsize_map_remove(gid_gsize_map_t *m, gid *key) {
  gid_gsize_map_assert_init(m);
  int64_t idx = gid_gsize_key_loc(m, key);
  if (idx == -1) return R_FAIL;
  _Bool true_flag = 1;
  m_bool_vec_put(&m->is_idx_open, idx, &true_flag);
  m->slots_in_use--;
  return R_OKAY;
}
int64_t gid_gsize_map_count_if(gid_gsize_map_t *m, gid_gsize_predicate f_pred) {
  gid_gsize_map_assert_init(m);
  int64_t counter = 0;
  for (int64_t i = 0; i < m->is_idx_open.length; i++) {
    if (!*m_bool_vec_at(&m->is_idx_open, i)) {
      if (f_pred(gid_gsize_map_item_vec_at(&m->map, i))) {
        counter++;
      }
    }
  }
  return counter;
}
void gid_gsize_map_foreach(gid_gsize_map_t *m, gid_gsize_predicate f_pred) {
  gid_gsize_map_assert_init(m);
  for (int64_t i = 0; i < m->is_idx_open.length; i++) {
    if (!*m_bool_vec_at(&m->is_idx_open, i)) {
      f_pred(gid_gsize_map_item_vec_at(&m->map, i));
    }
  }
}
any_vec_t *gid_gsize_map_filter(gid_gsize_map_t *m, any_vec_t *filter,
                                gid_gsize_predicate f_pred) {
  gid_gsize_map_assert_init(m);
  vec_unknown_type_init(filter, sizeof(gsize));
  for (int64_t i = 0; i < m->is_idx_open.length; i++) {
    if (!*m_bool_vec_at(&m->is_idx_open, i)) {
      gid_gsize_map_item *item = gid_gsize_map_item_vec_at(&m->map, i);
      if (f_pred(item)) {
        any_vec_push(filter, (void *)(&item->value));
      }
    }
  }
  return filter;
}
static void gid_gsize_map_assert_init(gid_gsize_map_t *m) {
  ((m) ? (void)(0)
       : __assert_fail("m", "/home/emil/GECS/src/core/register.c", 15,
                       __extension__ __PRETTY_FUNCTION__));
  ((m->is_idx_open.element_head)
       ? (void)(0)
       : __assert_fail("m->is_idx_open.element_head",
                       "/home/emil/GECS/src/core/register.c", 15,
                       __extension__ __PRETTY_FUNCTION__));
  ((m->map.element_head)
       ? (void)(0)
       : __assert_fail("m->map.element_head",
                       "/home/emil/GECS/src/core/register.c", 15,
                       __extension__ __PRETTY_FUNCTION__));
}
static uint64_t gid_gsize_hash(gid *m) {
  ((m) ? (void)(0)
       : __assert_fail("m", "/home/emil/GECS/src/core/register.c", 15,
                       __extension__ __PRETTY_FUNCTION__));
  return hash_bytes(m, sizeof(gid));
}
static void gid_gsize_map_lf_property(gid_gsize_map_t *m) {
  float lf = (float)m->slots_in_use / m->__size;
  if (lf < 0.75f) return;
  gid_gsize_map_t double_map;
  gid_gsize_map_init(&double_map);
  double_map.__size = m->__size * 2;
  m_bool_vec_resize(&double_map.is_idx_open, double_map.__size);
  gid_gsize_map_item_vec_resize(&double_map.map, double_map.__size);
  m_bool_vec_map(&double_map.is_idx_open, m_bool_set_to_true);
  for (int64_t slot_idx = 0; slot_idx < m->is_idx_open.length; slot_idx++) {
    if (!*m_bool_vec_at(&m->is_idx_open, slot_idx)) {
      gid_gsize_map_item *it = gid_gsize_map_item_vec_at(&m->map, slot_idx);
      gid_gsize_map_put(&double_map, &it->key, &it->value);
    }
  }
  gid_gsize_map_free(m);
  memmove(m, &double_map, sizeof(double_map));
}
static int64_t gid_gsize_key_loc(gid_gsize_map_t *m, gid *key) {
  gid_gsize_map_assert_init(m);
  uint64_t start_idx, idx;
  start_idx = gid_gsize_hash(key) % m->map.length;
  for (idx = start_idx; idx < (uint64_t)m->is_idx_open.length; idx++) {
    if (*m_bool_vec_at(&m->is_idx_open, idx)) continue;
    gid_gsize_map_item *it = gid_gsize_map_item_vec_at(&m->map, idx);
    if (memcmp(&it->key, key, sizeof(*key)) == 0) return idx;
  }
  for (idx = 0; idx < start_idx; idx++) {
    if (*m_bool_vec_at(&m->is_idx_open, idx)) continue;
    gid_gsize_map_item *it = gid_gsize_map_item_vec_at(&m->map, idx);
    if (memcmp(&it->key, key, sizeof(*key)) == 0) return idx;
  }
  return -1;
}
MAP_GEN_C(gstr, gint);

#include "set.h"

/*-------------------------------------------------------
 * Set Types
 *-------------------------------------------------------*/