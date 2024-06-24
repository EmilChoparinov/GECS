#include "logger.h"
#include "map.h"
#include "unity.h"
#include <stdbool.h>

typedef struct complex_struct cs;
struct complex_struct {
  int  x, y, z;
  bool active;
};

typedef struct id id;
struct id {
  int  uid;
  bool is_active;
} __attribute__((packed));

MAP_GEN_H(id, cs);
MAP_GEN_C(id, cs);

/*-------------------------------------------------------
 * REGISTER
 *-------------------------------------------------------*/

id_cs_map_t tmap;

void setUp(void) { id_cs_map_init(&tmap); }
void tearDown(void) { id_cs_map_free(&tmap); }

void test_put_and_find(void);
void test_has(void);
void test_remove(void);
void test_push_loadfactor(void);
void test_duplicate_handling(void);
void test_foreach(void);
void test_count_if(void);
void test_filter(void);
void test_to_vec(void);
void test_map_copy(void);

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_put_and_find);
  RUN_TEST(test_has);
  RUN_TEST(test_remove);
  RUN_TEST(test_push_loadfactor);
  RUN_TEST(test_duplicate_handling);
  RUN_TEST(test_foreach);
  RUN_TEST(test_count_if);
  RUN_TEST(test_filter);
  RUN_TEST(test_to_vec);
  RUN_TEST(test_map_copy);
  UNITY_END();
}

/*-------------------------------------------------------
 * TESTS
 *-------------------------------------------------------*/
void test_put_and_find(void) {
  id key = {.is_active = true, .uid = 69};
  id fake_key = {.is_active = true, .uid = 99};
  cs value = {.x = 99};
  TEST_ASSERT(id_cs_map_put(&tmap, &key, &value) == R_OKAY);

  TEST_ASSERT_MESSAGE(
      memcmp(&id_cs_map_find(&tmap, &key)->value, &value, sizeof(value)) == 0,
      "Did not retrieve from map value currently put in.");

  TEST_ASSERT_MESSAGE(id_cs_map_find(&tmap, &fake_key) == NULL,
                      "Key should not have returned");
}
void test_has(void) {
  id key = {.is_active = true, .uid = 69};
  id fake_key = {.is_active = true, .uid = 99};
  cs value = {.x = 99};
  TEST_ASSERT(id_cs_map_put(&tmap, &key, &value) == R_OKAY);

  TEST_ASSERT_MESSAGE(id_cs_map_has(&tmap, &key),
                      "Map should contain this keypair.");

  TEST_ASSERT_MESSAGE(!id_cs_map_has(&tmap, &fake_key),
                      "Map should NOT contain this keypair.");
}
void test_remove(void) {
  id key = {.is_active = true, .uid = 69};
  cs value = {.x = 99};
  TEST_ASSERT(id_cs_map_put(&tmap, &key, &value) == R_OKAY);

  TEST_ASSERT_MESSAGE(id_cs_map_has(&tmap, &key),
                      "Map should contain this keypair.");

  TEST_ASSERT(id_cs_map_remove(&tmap, &key) == R_OKAY);
  TEST_ASSERT_MESSAGE(!id_cs_map_has(&tmap, &key),
                      "Map should NOT contain this keypair.");
}

void test_push_loadfactor(void) {
  for (int i = 0; i < 500; i++) {
    TEST_ASSERT(id_cs_map_put(&tmap, &(id){.is_active = true, .uid = i},
                              &(cs){.x = 1, .y = 2, .z = 3, .active = true}) ==
                R_OKAY);
    TEST_ASSERT(id_cs_map_has(&tmap, &(id){.is_active = true, .uid = i}));
  }

  /* Check hash algo */
  for (int i = 0; i < 500; i++) {
    TEST_ASSERT(!id_cs_map_has(&tmap, &(id){.is_active = false, .uid = i}));
  }

  for (int i = 0; i < 500; i++) {
    TEST_ASSERT(id_cs_map_remove(&tmap, &(id){.is_active = true, .uid = i}) ==
                R_OKAY);
    TEST_ASSERT(!id_cs_map_has(&tmap, &(id){.is_active = true, .uid = i}));
  }

  TEST_ASSERT(tmap.slots_in_use == 0);
}

void test_duplicate_handling(void) {

  TEST_ASSERT(id_cs_map_put(&tmap, &(id){.is_active = true, .uid = 69},
                            &(cs){.x = 1, .y = 2, .z = 3, .active = true}) ==
              R_OKAY);
  for (int i = 0; i < 500; i++) {
    TEST_ASSERT(id_cs_map_put(&tmap, &(id){.is_active = true, .uid = 69},
                              &(cs){.x = 1, .y = 2, .z = 3, .active = true}) ==
                R_FAIL);
    TEST_ASSERT(tmap.slots_in_use == 1);
  }

  TEST_ASSERT(id_cs_map_remove(&tmap, &(id){.is_active = true, .uid = 69}) ==
              R_OKAY);
  for (int i = 0; i < 500; i++) {
    TEST_ASSERT(id_cs_map_remove(&tmap, &(id){.is_active = true, .uid = 69}) ==
                R_FAIL);
    TEST_ASSERT(tmap.slots_in_use == 0);
  }
}

bool is_active(id_cs_map_item *it, void *arg) {
  TEST_ASSERT(it->value.active);
  return it->value.active;
}
void test_foreach(void) {
  for (int i = 0; i < 500; i++) {
    TEST_ASSERT(id_cs_map_put(&tmap, &(id){.is_active = true, .uid = i},
                              &(cs){.x = 1, .y = 2, .z = 3, .active = true}) ==
                R_OKAY);
  }

  id_cs_map_foreach(&tmap, is_active, NULL);
}

void test_count_if(void) {
  for (int i = 0; i < 500; i++) {
    TEST_ASSERT(id_cs_map_put(&tmap, &(id){.is_active = true, .uid = i},
                              &(cs){.x = 1, .y = 2, .z = 3, .active = true}) ==
                R_OKAY);
  }

  TEST_ASSERT(id_cs_map_count_if(&tmap, is_active, NULL) == 500);
}

MAP_GEN_H(int, int);
// MAP_GEN_C(int, int);
static void int_int_map_item_assert_init_checks(int_int_map_item_vec_t *v);
static void int_int_map_item_bound_assert_checks(int_int_map_item_vec_t *v,
                                                 int64_t                 i);
static int_int_map_item_vec_t *
int_int_map_item_vec_construct(int_int_map_item_vec_t *v);
static int_int_map_item *int_int_map_item__d_vec_at(int_int_map_item_vec_t *v,
                                                    int64_t                 i);
int_int_map_item_vec_t *
int_int_map_item_vec_construct(int_int_map_item_vec_t *v) {
  v->length = 0;
  v->__size = 1;
  v->__top = 0;
  return v;
}
int_int_map_item_vec_t *int_int_map_item_vec_init(int_int_map_item_vec_t *v) {
  int_int_map_item_vec_construct(v);
  v->__el_size = sizeof(int_int_map_item);
  v->element_head = calloc(v->__size, v->__el_size);
  ((v->element_head)
       ? (void)(0)
       : __assert_fail("v->element_head", "/home/emil/GECS/tests/map_tests.c",
                       168, __extension__ __PRETTY_FUNCTION__));
  return v;
}
retcode int_int_map_item_vec_free(int_int_map_item_vec_t *v) {
  ((v) ? (void)(0)
       : __assert_fail("v", "/home/emil/GECS/tests/map_tests.c", 168,
                       __extension__ __PRETTY_FUNCTION__));
  ((v->element_head != ((void *)0))
       ? (void)(0)
       : __assert_fail("v->element_head != NULL",
                       "/home/emil/GECS/tests/map_tests.c", 168,
                       __extension__ __PRETTY_FUNCTION__));
  free(v->element_head);
  return R_OKAY;
}
int_int_map_item_vec_t *int_int_map_item_vec_heap_init(void) {
  int_int_map_item_vec_t *v = malloc(sizeof(*v));
  ((v) ? (void)(0)
       : __assert_fail("v", "/home/emil/GECS/tests/map_tests.c", 168,
                       __extension__ __PRETTY_FUNCTION__));
  int_int_map_item_vec_init(v);
  return v;
}
void int_int_map_item_vec_heap_free(int_int_map_item_vec_t *v) {
  int_int_map_item_vec_free(v);
  free(v);
}
retcode int_int_map_item_vec_resize(int_int_map_item_vec_t *v, int64_t size) {
  int_int_map_item_assert_init_checks(v);
  v->length = size;
  if (size < v->__size) return R_OKAY;
  int64_t old_size = v->__size;
  while (size >= v->__size) v->__size *= 2;
  void *new_addr = calloc(v->__size * v->__el_size, 1);
  ((new_addr) ? (void)(0)
              : __assert_fail("new_addr", "/home/emil/GECS/tests/map_tests.c",
                              168, __extension__ __PRETTY_FUNCTION__));
  memcpy(new_addr, v->element_head, old_size * v->__el_size);
  free(v->element_head);
  v->element_head = new_addr;
  return R_OKAY;
}
int_int_map_item *int_int_map_item_vec_at(int_int_map_item_vec_t *v,
                                          int64_t                 i) {
  int_int_map_item_bound_assert_checks(v, i);
  return int_int_map_item__d_vec_at(v, i);
}
retcode int_int_map_item_vec_put(int_int_map_item_vec_t *v, int64_t i,
                                 int_int_map_item *element) {
  int_int_map_item_bound_assert_checks(v, i);
  ((element) ? (void)(0)
             : __assert_fail("element", "/home/emil/GECS/tests/map_tests.c",
                             168, __extension__ __PRETTY_FUNCTION__));
  memmove((char *)v->element_head + i * v->__el_size, element, v->__el_size);
  return R_OKAY;
}
retcode int_int_map_item_vec_delete_at(int_int_map_item_vec_t *v, int64_t idx) {
  int_int_map_item_assert_init_checks(v);
  if (v->length - 1 == idx) {
    int_int_map_item_vec_pop(v);
    return R_OKAY;
  }
  void *loc = (char *)v->element_head + idx * v->__el_size;
  void *loc_next = (char *)v->element_head + (idx + 1) * v->__el_size;
  memmove(loc, loc_next, v->length * v->__el_size - idx - v->__el_size);
  v->length--;
  v->__top--;
  return R_OKAY;
}
retcode int_int_map_item_vec_delete(int_int_map_item_vec_t *v,
                                    int_int_map_item       *el) {
  int_int_map_item_assert_init_checks(v);
  int64_t idx = int_int_map_item_vec_find(v, el);
  ((idx != -1) ? (void)(0)
               : __assert_fail("idx != -1", "/home/emil/GECS/tests/map_tests.c",
                               168, __extension__ __PRETTY_FUNCTION__));
  if (idx == v->length - 1) {
    int_int_map_item_vec_pop(v);
    return R_OKAY;
  }
  return int_int_map_item_vec_delete_at(v, idx);
}
_Bool int_int_map_item_vec_has(int_int_map_item_vec_t *v,
                               int_int_map_item       *element) {
  int_int_map_item_assert_init_checks(v);
  ((element) ? (void)(0)
             : __assert_fail("element", "/home/emil/GECS/tests/map_tests.c",
                             168, __extension__ __PRETTY_FUNCTION__));
  return int_int_map_item_vec_find(v, element) != -1;
}
int64_t int_int_map_item_vec_find(int_int_map_item_vec_t *v,
                                  int_int_map_item       *element) {
  int_int_map_item_assert_init_checks(v);
  ((element) ? (void)(0)
             : __assert_fail("element", "/home/emil/GECS/tests/map_tests.c",
                             168, __extension__ __PRETTY_FUNCTION__));
  for (int64_t i = 0; i < v->length; i++)
    if (memcmp(element, int_int_map_item_vec_at(v, i), v->__el_size) == 0)
      return i;
  return -1;
}
retcode int_int_map_item_vec_push(int_int_map_item_vec_t *v,
                                  int_int_map_item       *element) {
  int_int_map_item_assert_init_checks(v);
  ((element) ? (void)(0)
             : __assert_fail("element", "/home/emil/GECS/tests/map_tests.c",
                             168, __extension__ __PRETTY_FUNCTION__));
  _Bool ret = int_int_map_item_vec_resize(v, v->length) == R_OKAY;
  ((ret) ? (void)(0)
         : __assert_fail("ret", "/home/emil/GECS/tests/map_tests.c", 168,
                         __extension__ __PRETTY_FUNCTION__));
  memmove(int_int_map_item__d_vec_at(v, v->__top), element, v->__el_size);
  v->__top++;
  if (v->__top >= v->length) v->length = v->__top;
  return ret;
}
void int_int_map_item_vec_pop(int_int_map_item_vec_t *v) {
  int_int_map_item_assert_init_checks(v);
  ((v->__top) ? (void)(0)
              : __assert_fail("v->__top", "/home/emil/GECS/tests/map_tests.c",
                              168, __extension__ __PRETTY_FUNCTION__));
  if (v->__top == v->length) v->length--;
  v->__top--;
}
int_int_map_item *int_int_map_item_vec_top(int_int_map_item_vec_t *v) {
  int_int_map_item_assert_init_checks(v);
  return int_int_map_item_vec_at(v, v->__top - 1);
}
retcode int_int_map_item_vec_swap(int_int_map_item_vec_t *v, int64_t a,
                                  int64_t b) {
  int_int_map_item *a_data = int_int_map_item_vec_at(v, a);
  int_int_map_item *b_data = int_int_map_item_vec_at(v, b);
  memswap(a_data, b_data, sizeof(int_int_map_item));
  return R_OKAY;
}
retcode int_int_map_item_vec_copy(int_int_map_item_vec_t *dest,
                                  int_int_map_item_vec_t *src) {
  int_int_map_item_assert_init_checks(dest);
  int_int_map_item_assert_init_checks(src);
  int_int_map_item_vec_free(dest);
  memmove(dest, src, sizeof(*src));
  if (dest->length == 0) return R_OKAY;
  dest->element_head = malloc(src->__size * src->__el_size);
  memmove(dest->element_head, src->element_head, src->__size * src->__el_size);
  return R_OKAY;
}
retcode int_int_map_item_vec_clear(int_int_map_item_vec_t *v) {
  int_int_map_item_assert_init_checks(v);
  v->length = 0;
  v->__top = 0;
  return R_OKAY;
}
retcode int_int_map_item_vec_sort(int_int_map_item_vec_t  *v,
                                  int_int_map_item_compare cmp, void *arg) {
  int_int_map_item_assert_init_checks(v);
  int64_t i, j;
  for (i = 0; i < v->length; i++) {
    for (j = 0; j < v->length - i - 1; j++) {
      if (!cmp(int_int_map_item_vec_at(v, j), int_int_map_item_vec_at(v, j + 1),
               arg))
        int_int_map_item_vec_swap(v, j, j + 1);
    }
  }
  return R_OKAY;
}
int64_t int_int_map_item_vec_count_if(int_int_map_item_vec_t    *v,
                                      int_int_map_item_predicate f_pred,
                                      void                      *arg) {
  int_int_map_item_assert_init_checks(v);
  int64_t counter = 0;
  for (int64_t i = 0; i < v->length; i++)
    if (f_pred(&v->element_head[i], arg)) counter++;
  return counter;
}
int_int_map_item_vec_t *
int_int_map_item_vec_filter(int_int_map_item_vec_t    *v,
                            int_int_map_item_predicate f_pred, void *arg) {
  int_int_map_item_assert_init_checks(v);
  int_int_map_item_vec_t filter;
  int_int_map_item_vec_init(&filter);
  for (int64_t i = 0; i < v->length; i++)
    if (f_pred(&v->element_head[i], arg))
      int_int_map_item_vec_push(&filter, &v->element_head[i]);
  int_int_map_item_vec_free(v);
  memmove(v, &filter, sizeof(*v));
  return v;
}
void int_int_map_item_vec_foreach(int_int_map_item_vec_t    *v,
                                  int_int_map_item_predicate f_pred,
                                  void                      *arg) {
  int_int_map_item_assert_init_checks(v);
  for (int64_t i = 0; i < v->length; i++) f_pred(&v->element_head[i], arg);
}
int_int_map_item_vec_t *int_int_map_item_vec_map(int_int_map_item_vec_t *v,
                                                 int_int_map_item_unary f_unary,
                                                 void                  *arg) {
  int_int_map_item_assert_init_checks(v);
  for (int64_t i = 0; i < v->length; i++)
    v->element_head[i] = f_unary(v->element_head[i], arg);
  return v;
}
int_int_map_item int_int_map_item_vec_foldl(int_int_map_item_vec_t *v,
                                            int_int_map_item_binary f_binary,
                                            int_int_map_item start, void *arg) {
  int_int_map_item_assert_init_checks(v);
  int_int_map_item result = start;
  for (int64_t i = 0; i < v->length; i++)
    result = f_binary(result, &v->element_head[i], arg);
  return result;
}
static void int_int_map_item_assert_init_checks(int_int_map_item_vec_t *v) {
  ((v) ? (void)(0)
       : __assert_fail("v", "/home/emil/GECS/tests/map_tests.c", 168,
                       __extension__ __PRETTY_FUNCTION__));
  ((v->element_head)
       ? (void)(0)
       : __assert_fail("v->element_head", "/home/emil/GECS/tests/map_tests.c",
                       168, __extension__ __PRETTY_FUNCTION__));
}
static void int_int_map_item_bound_assert_checks(int_int_map_item_vec_t *v,
                                                 int64_t                 i) {
  int_int_map_item_assert_init_checks(v);
  ((i >= 0 && i < v->length)
       ? (void)(0)
       : __assert_fail("i >= 0 && i < v->length",
                       "/home/emil/GECS/tests/map_tests.c", 168,
                       __extension__ __PRETTY_FUNCTION__));
}
static int_int_map_item *int_int_map_item__d_vec_at(int_int_map_item_vec_t *v,
                                                    int64_t                 i) {
  return (void *)(v->element_head) + i * v->__el_size;
};
static uint64_t int_int_hash(int *m);
static int64_t  int_int_key_loc(int_int_map_t *m, int *key);
static void     int_int_map_assert_init(int_int_map_t *m);
static void     int_int_map_lf_property(int_int_map_t *m);
int_int_map_t  *int_int_map_init(int_int_map_t *m) {
  m->__size = 16;
  m->slots_in_use = 0;
  int_int_map_item_vec_init(&m->map);
  int_int_map_item_vec_resize(&m->map, m->__size);
  m_bool_vec_init(&m->is_idx_open);
  m_bool_vec_resize(&m->is_idx_open, m->__size);
  m_bool_vec_map(&m->is_idx_open, m_bool_set_to_true, ((void *)0));
  return m;
}
int_int_map_t *int_int_map_copy(int_int_map_t *dst, int_int_map_t *src) {
  int_int_map_assert_init(src);
  memmove(dst, src, sizeof(*src));
  m_bool_vec_init(&dst->is_idx_open);
  int_int_map_item_vec_init(&dst->map);
  m_bool_vec_copy(&dst->is_idx_open, &src->is_idx_open);
  int_int_map_item_vec_copy(&dst->map, &src->map);
  return dst;
}
void int_int_map_free(int_int_map_t *m) {
  int_int_map_assert_init(m);
  int_int_map_item_vec_free(&m->map);
  m_bool_vec_free(&m->is_idx_open);
}
int_int_map_t *int_int_map_heap_init(void) {
  int_int_map_t *m = malloc(sizeof(*m));
  ((m) ? (void)(0)
       : __assert_fail("m", "/home/emil/GECS/tests/map_tests.c", 168,
                       __extension__ __PRETTY_FUNCTION__));
  int_int_map_init(m);
  return m;
}
void int_int_map_clear(int_int_map_t *m) {
  m->slots_in_use = 0;
  int_int_map_item_vec_clear(&m->map);
  int_int_map_item_vec_resize(&m->map, m->__size);
  m_bool_vec_clear(&m->is_idx_open);
  m_bool_vec_resize(&m->is_idx_open, m->__size);
  m_bool_vec_map(&m->is_idx_open, m_bool_set_to_true, ((void *)0));
}
void int_int_map_heap_free(int_int_map_t *m) {
  int_int_map_assert_init(m);
  m->__size = 16;
  m->slots_in_use = 0;
  int_int_map_free(m);
  free(m);
}
any_vec_t *int_int_map_to_vec(int_int_map_t *m, any_vec_t *v) {
  return int_int_map_filter(m, v, (int_int_predicate)m_bool_set_to_true,
                            ((void *)0));
}
any_vec_t *int_int_mmap_to_vec(int_int_map_t *m, any_vec_t *v) {
  return int_int_mmap_filter(m, v, (int_int_predicate)m_bool_set_to_true,
                             ((void *)0));
}
any_vec_t *int_int_map_ptrs(int_int_map_t *m, any_vec_t *v) {
  int_int_map_assert_init(m);
  vec_unknown_type_init(v, sizeof(void *));
  for (int64_t i = 0; i < m->is_idx_open.length; i++) {
    if (!*m_bool_vec_at(&m->is_idx_open, i)) {
      int_int_map_item *item = int_int_map_item_vec_at(&m->map, i);
      void             *p = &item->value;
      any_vec_push(v, &p);
    }
  }
  return v;
}
int_int_map_item *int_int_map_find(int_int_map_t *m, int *key) {
  int_int_map_assert_init(m);
  int64_t idx = int_int_key_loc(m, key);
  if (idx != -1) return int_int_map_item_vec_at(&m->map, idx);
  return ((void *)0);
}
retcode int_int_map_put(int_int_map_t *m, int *key, int *value) {
  int_int_map_item item = {0};
  memmove(&item.key, key, sizeof(*key));
  memmove(&item.value, value, sizeof(*value));
  int_int_map_assert_init(m);
  int_int_map_lf_property(m);
  uint64_t start_idx, idx;
  _Bool    false_flag = 0;
  start_idx = int_int_hash(key) % m->map.length;
  for (idx = start_idx; idx < (uint64_t)m->map.length; idx++) {
    if (*m_bool_vec_at(&m->is_idx_open, idx)) {
      m_bool_vec_put(&m->is_idx_open, idx, &false_flag);
      m->slots_in_use++;
      int_int_map_item_vec_put(&m->map, idx, &item);
      return R_OKAY;
    }
    if (memcmp(&int_int_map_item_vec_at(&m->map, idx)->key, key,
               sizeof(*key)) == 0)
      return R_FAIL;
  }
  for (idx = 0; idx < start_idx; idx++) {
    if (*m_bool_vec_at(&m->is_idx_open, idx)) {
      m_bool_vec_put(&m->is_idx_open, idx, &false_flag);
      m->slots_in_use++;
      int_int_map_item_vec_put(&m->map, idx, &item);
      return R_OKAY;
    }
    if (memcmp(&int_int_map_item_vec_at(&m->map, idx)->key, key,
               sizeof(*key)) == 0)
      return R_FAIL;
  }
  return R_FAIL;
}
_Bool int_int_map_has(int_int_map_t *m, int *key) {
  return int_int_map_find(m, key) != ((void *)0);
}
retcode int_int_map_remove(int_int_map_t *m, int *key) {
  int_int_map_assert_init(m);
  int64_t idx = int_int_key_loc(m, key);
  if (idx == -1) return R_FAIL;
  _Bool true_flag = 1;
  m_bool_vec_put(&m->is_idx_open, idx, &true_flag);
  m->slots_in_use--;
  return R_OKAY;
}
int64_t int_int_map_count_if(int_int_map_t *m, int_int_predicate f_pred,
                             void *arg) {
  int_int_map_assert_init(m);
  int64_t counter = 0;
  for (int64_t i = 0; i < m->is_idx_open.length; i++) {
    if (!*m_bool_vec_at(&m->is_idx_open, i)) {
      if (f_pred(int_int_map_item_vec_at(&m->map, i), arg)) {
        counter++;
      }
    }
  }
  return counter;
}
void int_int_map_foreach(int_int_map_t *m, int_int_predicate f_pred,
                         void *arg) {
  int_int_map_assert_init(m);
  for (int64_t i = 0; i < m->is_idx_open.length; i++) {
    if (!*m_bool_vec_at(&m->is_idx_open, i)) {
      f_pred(int_int_map_item_vec_at(&m->map, i), arg);
    }
  }
}
any_vec_t *int_int_map_filter(int_int_map_t *m, any_vec_t *filter,
                              int_int_predicate f_pred, void *arg) {
  int_int_map_assert_init(m);
  vec_unknown_type_init(filter, sizeof(int));
  for (int64_t i = 0; i < m->is_idx_open.length; i++) {
    if (!*m_bool_vec_at(&m->is_idx_open, i)) {
      int_int_map_item *item = int_int_map_item_vec_at(&m->map, i);
      if (f_pred(item, arg)) {
        any_vec_push(filter, (void *)&item->value);
      }
    }
  }
  return filter;
}
any_vec_t *int_int_mmap_filter(int_int_map_t *m, any_vec_t *filter,
                               int_int_predicate f_pred, void *arg) {
  int_int_map_assert_init(m);
  vec_unknown_type_init(filter, sizeof(void *));
  for (int64_t i = 0; i < m->is_idx_open.length; i++) {
    if (!*m_bool_vec_at(&m->is_idx_open, i)) {
      int_int_map_item *item = int_int_map_item_vec_at(&m->map, i);
      if (f_pred(item, arg)) {
        void *value = &item->value;
        any_vec_push(filter, &value);
      }
    }
  }
  return filter;
}
int_int_map_item *int_int_ffind_one(int_int_map_t *m, int_int_predicate f_pred,
                                    void *arg) {
  int_int_map_assert_init(m);
  for (int64_t i = 0; i < m->is_idx_open.length; i++) {
    if (!*m_bool_vec_at(&m->is_idx_open, i)) {
      int_int_map_item *item = int_int_map_item_vec_at(&m->map, i);
      if (f_pred(item, arg)) return item;
    }
  }
  return ((void *)0);
}
static void int_int_map_assert_init(int_int_map_t *m) {
  ((m) ? (void)(0)
       : __assert_fail("m", "/home/emil/GECS/tests/map_tests.c", 168,
                       __extension__ __PRETTY_FUNCTION__));
  ((m->is_idx_open.element_head)
       ? (void)(0)
       : __assert_fail("m->is_idx_open.element_head",
                       "/home/emil/GECS/tests/map_tests.c", 168,
                       __extension__ __PRETTY_FUNCTION__));
  ((m->map.element_head)
       ? (void)(0)
       : __assert_fail("m->map.element_head",
                       "/home/emil/GECS/tests/map_tests.c", 168,
                       __extension__ __PRETTY_FUNCTION__));
}
static uint64_t int_int_hash(int *m) {
  ((m) ? (void)(0)
       : __assert_fail("m", "/home/emil/GECS/tests/map_tests.c", 168,
                       __extension__ __PRETTY_FUNCTION__));
  return hash_bytes(m, sizeof(int));
}
static void int_int_map_lf_property(int_int_map_t *m) {
  float lf = (float)m->slots_in_use / m->__size;
  if (lf < 0.75f) return;
  int_int_map_t double_map;
  int_int_map_init(&double_map);
  double_map.__size = m->__size * 2;
  m_bool_vec_resize(&double_map.is_idx_open, double_map.__size);
  int_int_map_item_vec_resize(&double_map.map, double_map.__size);
  m_bool_vec_map(&double_map.is_idx_open, m_bool_set_to_true, ((void *)0));
  for (int64_t slot_idx = 0; slot_idx < m->is_idx_open.length; slot_idx++) {
    if (!*m_bool_vec_at(&m->is_idx_open, slot_idx)) {
      int_int_map_item *it = int_int_map_item_vec_at(&m->map, slot_idx);
      int_int_map_put(&double_map, &it->key, &it->value);
    }
  }
  int_int_map_free(m);
  memmove(m, &double_map, sizeof(double_map));
}
static int64_t int_int_key_loc(int_int_map_t *m, int *key) {
  int_int_map_assert_init(m);
  uint64_t start_idx, idx;
  start_idx = int_int_hash(key) % m->map.length;
  for (idx = start_idx; idx < (uint64_t)m->is_idx_open.length; idx++) {
    if (*m_bool_vec_at(&m->is_idx_open, idx)) continue;
    int_int_map_item *it = int_int_map_item_vec_at(&m->map, idx);
    if (memcmp(&it->key, key, sizeof(*key)) == 0) return idx;
  }
  for (idx = 0; idx < start_idx; idx++) {
    if (*m_bool_vec_at(&m->is_idx_open, idx)) continue;
    int_int_map_item *it = int_int_map_item_vec_at(&m->map, idx);
    if (memcmp(&it->key, key, sizeof(*key)) == 0) return idx;
  }
  return -1;
}
bool select_filter(int_int_map_item *t, void *arg) {
  return t->value == 1 || t->value == 10;
}

void test_filter(void) {
  int_int_map_t map;
  int_int_map_init(&map);

  for (int i = 0; i < 500; i++) {
    TEST_ASSERT(int_int_map_put(&map, &i, &i) == R_OKAY);
  }

  // filter selects those where x = 1 and x = 10
  any_vec_t v;
  int_int_map_filter(&map, &v, select_filter, NULL);

  int i = 1;
  TEST_ASSERT(any_vec_has(&v, of_any(&i)));
  i = 10;
  TEST_ASSERT(any_vec_has(&v, of_any(&i)));
  i = 501;
  TEST_ASSERT(!any_vec_has(&v, of_any(&i)));

  TEST_ASSERT(v.length == 2);
}

void test_to_vec(void) {
  int_int_map_t map;
  int_int_map_init(&map);

  for (int i = 0; i < 500; i++) {
    TEST_ASSERT(int_int_map_put(&map, &i, &i) == R_OKAY);
  }

  any_vec_t copy;
  int_int_map_to_vec(&map, &copy);

  for (int i = 0; i < 500; i++) {
    TEST_ASSERT(any_vec_has(&copy, of_any(&i)));
  }
}

void test_map_copy(void) {
  int_int_map_t orig, copy;
  int_int_map_init(&orig);

  for (int i = 0; i < 50; i++) {
    int_int_map_put(&orig, &i, &i);
  }

  int_int_map_copy(&copy, &orig);

  /* Assert pointers of memory go to different segments */
  TEST_ASSERT(copy.is_idx_open.element_head != orig.is_idx_open.element_head);
  TEST_ASSERT(copy.map.element_head != orig.map.element_head);

  /* Assert maps are diverging  */
  int x = 51;
  int_int_map_put(&orig, &x, &x);

  TEST_ASSERT(orig.slots_in_use == copy.slots_in_use + 1);
}
