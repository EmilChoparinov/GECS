/* =========================================================================
    Author: E.D Choparinov, Amsterdam
    Related Files: map.h
    Created On: June 01 2024
    Purpose:
        The purpose of this file is to provide macro generation code for
        generic arraylike structures.
========================================================================= */

#ifndef __HEADER_MAP_H__
#define __HEADER_MAP_H__

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

#include "funcdef.h"
#include "gecs_codes.h"

#include "vector.h" /* This implementation relies on the vector structure so 
                       file is **NOT** standalone. */

/* This implemenation relies on boolean vectors to keep track of slots. To
   maintain cleanliness this private boolean alias will be made and used
   instead. */
typedef bool m_bool;
VECTOR_GEN_H(m_bool);
VECTOR_GEN_C(m_bool);

#define MAP_DEFAULT_SIZE 16
#define MAP_LOAD_FACTOR  0.75f

#define map_pair(Ta, Tb)       Ta##_##Tb
#define map_t(Ta, Tb)          Ta##_##Tb##_map_t
#define map_item_t(Ta, Tb)     Ta##_##Tb##_map_item
#define map_func(Ta, Tb, func) func(Ta##_##Tb)

#define ffname(Ta, Tb, name)     fname(Ta##_##Tb, name)
#define map_vec_t(Ta, Tb)        Ta##_##Tb##_map_item_vec_t
#define map_access(Ta, Tb, func) Ta##_##Tb##_map_item_vec_##func

#define MAP_GEN_H(Ta, Tb)                                                        \
  /*-------------------------------------------------------                      \
   * Define Datastructures                                                       \
   *-------------------------------------------------------*/                    \
  typedef struct map_t(Ta, Tb) map_t(Ta, Tb);                                    \
  typedef struct map_item_t(Ta, Tb) map_item_t(Ta, Tb);                          \
                                                                                 \
  struct map_item_t(Ta, Tb) {                                                    \
    Ta key;                                                                      \
    Tb value;                                                                    \
  };                                                                             \
  VECTOR_GEN_H(map_item_t(Ta, Tb));                                              \
                                                                                 \
  struct map_t(Ta, Tb) {                                                         \
    int64_t      __size; /* The allocated count of elements, used internally. */ \
    int64_t      slots_in_use; /* The count of elements in the map. */           \
    m_bool_vec_t is_idx_open;                                                    \
    map_vec_t(Ta, Tb) map;                                                       \
  };                                                                             \
                                                                                 \
  /*-------------------------------------------------------                      \
   * Define Comparator Types                                                     \
   *-------------------------------------------------------*/                    \
  typedef bool (*map_func(Ta, Tb, pred_f))(map_item_t(Ta, Tb) * a);              \
  typedef map_item_t(Ta, Tb) (*map_func(Ta, Tb, unary_f))(map_item_t(Ta, Tb));   \
                                                                                 \
  /*-------------------------------------------------------                      \
   * Container Operations                                                        \
   *-------------------------------------------------------*/                    \
  fdecl(map_t(Ta, Tb) *, map_pair(Ta, Tb), map_init, (map_t(Ta, Tb) * m));       \
  fdecl(void, map_pair(Ta, Tb), map_free, (map_t(Ta, Tb) * m));                  \
  fdecl(map_t(Ta, Tb) *, map_pair(Ta, Tb), map_heap_init, (void));               \
  fdecl(void, map_pair(Ta, Tb), map_heap_free, (map_t(Ta, Tb) * m));             \
                                                                                 \
  /*------------------------------------------------------- \                    \
   * Element Operations                                                          \
   *-------------------------------------------------------*/                    \
  fdecl(map_item_t(Ta, Tb) *, map_pair(Ta, Tb), map_find,                        \
        (map_t(Ta, Tb) * m, Ta * key));                                          \
  fdecl(retcode, map_pair(Ta, Tb), map_put,                                      \
        (map_t(Ta, Tb) * m, Ta * key, Tb * value));                              \
  fdecl(bool, map_pair(Ta, Tb), map_has, (map_t(Ta, Tb) * m, Ta * key));         \
  fdecl(retcode, map_pair(Ta, Tb), map_remove, (map_t(Ta, Tb) * m, Ta * key));   \
                                                                                 \
  /*-------------------------------------------------------                      \
   * Functional Operations                                                       \
   *-------------------------------------------------------*/                    \
  fdecl(int64_t, map_pair(Ta, Tb), map_count_if,                                 \
        (map_t(Ta, Tb) * m, map_func(Ta, Tb, pred_f) f_pred));

#define MAP_GEN_C(Ta, Tb)                                                      \
  /* This is ok because its an "internal" datastructure type. The user doesn't \
     and shouldn't know/access the internal vecture structure generated. */    \
  VECTOR_GEN_C(map_item_t(Ta, Tb));                                            \
  static ret(uint64_t) ffname(Ta, Tb, hash)(Ta * m);                           \
  static ret(int64_t) ffname(Ta, Tb, key_loc)(map_t(Ta, Tb) * m, Ta * key);    \
  static ret(void) ffname(Ta, Tb, map_assert_init)(map_t(Ta, Tb) * m);         \
  static ret(void) ffname(Ta, Tb, map_lf_property)(map_t(Ta, Tb) * m);         \
                                                                               \
  m_bool m_bool_set_to_true(m_bool b) { return true; }                         \
  /*-------------------------------------------------------                    \
   * Container Operations                                                      \
   *-------------------------------------------------------*/                  \
  ret(map_t(Ta, Tb) *) ffname(Ta, Tb, map_init)(map_t(Ta, Tb) * m) {           \
    m->__size = MAP_DEFAULT_SIZE;                                              \
    m->slots_in_use = 0;                                                       \
                                                                               \
    map_access(Ta, Tb, init)(&m->map);                                         \
    map_access(Ta, Tb, resize)(&m->map, m->__size);                            \
                                                                               \
    m_bool_vec_init(&m->is_idx_open);                                          \
    m_bool_vec_resize(&m->is_idx_open, m->__size);                             \
    m_bool_vec_map(&m->is_idx_open, m_bool_set_to_true);                       \
                                                                               \
    return m;                                                                  \
  }                                                                            \
                                                                               \
  ret(void) ffname(Ta, Tb, map_free)(map_t(Ta, Tb) * m) {                      \
    ffname(Ta, Tb, map_assert_init)(m);                                        \
                                                                               \
    map_access(Ta, Tb, free)(&m->map);                                         \
    m_bool_vec_free(&m->is_idx_open);                                          \
  }                                                                            \
                                                                               \
  ret(map_t(Ta, Tb) *) ffname(Ta, Tb, map_heap_init)(void) {                   \
    map_t(Ta, Tb) *m = malloc(sizeof(*m));                                     \
    assert(m);                                                                 \
                                                                               \
    ffname(Ta, Tb, map_init)(m);                                               \
                                                                               \
    return m;                                                                  \
  }                                                                            \
                                                                               \
  ret(void) ffname(Ta, Tb, map_heap_free)(map_t(Ta, Tb) * m) {                 \
    ffname(Ta, Tb, map_assert_init)(m);                                        \
    m->__size = MAP_DEFAULT_SIZE;                                              \
    m->slots_in_use = 0;                                                       \
                                                                               \
    ffname(Ta, Tb, map_free)(m);                                               \
    free(m);                                                                   \
  }                                                                            \
                                                                               \
  /*-------------------------------------------------------                    \
   * Element Operations                                                        \
   *-------------------------------------------------------*/                  \
  ret(map_item_t(Ta, Tb) *)                                                    \
      ffname(Ta, Tb, map_find)(map_t(Ta, Tb) * m, Ta * key) {                  \
    ffname(Ta, Tb, map_assert_init)(m);                                        \
                                                                               \
    int64_t idx = ffname(Ta, Tb, key_loc)(m, key);                             \
    if (idx != -1) return map_access(Ta, Tb, at)(&m->map, idx);                \
                                                                               \
    return NULL;                                                               \
  }                                                                            \
                                                                               \
  ret(retcode)                                                                 \
      ffname(Ta, Tb, map_put)(map_t(Ta, Tb) * m, Ta * key, Tb * value) {       \
    map_item_t(Ta, Tb) item = {0};                                             \
    memmove(&item.key, key, sizeof(*key));                                     \
    memmove(&item.value, value, sizeof(*value));                               \
    ffname(Ta, Tb, map_assert_init)(m);                                        \
    ffname(Ta, Tb, map_lf_property)(m);                                        \
    uint64_t start_idx, idx;                                                   \
    bool     false_flag = false;                                               \
                                                                               \
    start_idx = ffname(Ta, Tb, hash)(key) % m->map.length;                     \
    /* map->length and is_idx_open->length are equal at all times. */          \
    for (idx = start_idx; idx < (uint64_t)m->map.length; idx++) {              \
                                                                               \
      /* If open, take slot  */                                                \
      if (*m_bool_vec_at(&m->is_idx_open, idx)) {                              \
        m_bool_vec_put(&m->is_idx_open, idx, &false_flag);                     \
        m->slots_in_use++;                                                     \
        map_access(Ta, Tb, put)(&m->map, idx, &item);                          \
        return R_OKAY;                                                         \
      }                                                                        \
      /* We need to check slots as we go to prevent duplicates. */             \
      if (memcmp(&map_access(Ta, Tb, at)(&m->map, idx)->key, key,              \
                 sizeof(*key)) == 0)                                           \
        return R_FAIL;                                                         \
    }                                                                          \
                                                                               \
    /* Probe failed, go to zero and probe to start_idx. */                     \
    for (idx = 0; idx < start_idx; idx++) {                                    \
      if (*m_bool_vec_at(&m->is_idx_open, idx)) {                              \
        m_bool_vec_put(&m->is_idx_open, idx, &false_flag);                     \
        m->slots_in_use++;                                                     \
        map_access(Ta, Tb, put)(&m->map, idx, &item);                          \
        return R_OKAY;                                                         \
      }                                                                        \
                                                                               \
      if (memcmp(&map_access(Ta, Tb, at)(&m->map, idx)->key, key,              \
                 sizeof(*key)) == 0)                                           \
        return R_FAIL;                                                         \
    }                                                                          \
                                                                               \
    /* This should be impossible to reach due to the load factor constraint.   \
     */                                                                        \
    return R_FAIL;                                                             \
  }                                                                            \
                                                                               \
  ret(bool) ffname(Ta, Tb, map_has)(map_t(Ta, Tb) * m, Ta * key) {             \
    return ffname(Ta, Tb, map_find)(m, key) != NULL;                           \
  }                                                                            \
                                                                               \
  ret(retcode) ffname(Ta, Tb, map_remove)(map_t(Ta, Tb) * m, Ta * key) {       \
    ffname(Ta, Tb, map_assert_init)(m);                                        \
                                                                               \
    int64_t idx = ffname(Ta, Tb, key_loc)(m, key);                             \
    if (idx == -1) return R_FAIL;                                              \
                                                                               \
    bool true_flag = true;                                                     \
    m_bool_vec_put(&m->is_idx_open, idx, &true_flag);                          \
    m->slots_in_use--;                                                         \
                                                                               \
    return R_OKAY;                                                             \
  }                                                                            \
                                                                               \
  /*-------------------------------------------------------                    \
   * Functional Operations                                                     \
   *-------------------------------------------------------*/                  \
  ret(int64_t) ffname(Ta, Tb, map_count_if)(map_t(Ta, Tb) * m,                 \
                                            map_func(Ta, Tb, pred_f) f_pred) { \
    ffname(Ta, Tb, map_assert_init)(m);                                        \
    int64_t counter = 0;                                                       \
    for (int64_t i = 0; i < m->is_idx_open.length; i++) {                      \
      if (!*m_bool_vec_at(&m->is_idx_open, i)) {                               \
        if (f_pred(map_access(Ta, Tb, at)(&m->map, i))) {                      \
          counter++;                                                           \
        }                                                                      \
      }                                                                        \
    }                                                                          \
    return counter;                                                            \
  }                                                                            \
                                                                               \
  /*-------------------------------------------------------                    \
   * Static Functions                                                          \
   *-------------------------------------------------------*/                  \
  static ret(void) ffname(Ta, Tb, map_assert_init)(map_t(Ta, Tb) * m) {        \
    assert(m);                                                                 \
    assert(m->is_idx_open.element_head);                                       \
    assert(m->map.element_head);                                               \
  }                                                                            \
                                                                               \
  static ret(uint64_t) ffname(Ta, Tb, hash)(Ta * m) {                          \
    assert(m);                                                                 \
    return hash_bytes(m, sizeof(Ta));                                          \
  }                                                                            \
                                                                               \
  static ret(void) ffname(Ta, Tb, map_lf_property)(map_t(Ta, Tb) * m) {        \
    float lf = (float)m->slots_in_use / m->__size;                             \
    if (lf < MAP_LOAD_FACTOR) return;                                          \
                                                                               \
    /* Create a temporary map container to move all elements into */           \
    map_t(Ta, Tb) double_map;                                                  \
    ffname(Ta, Tb, map_init)(&double_map);                                     \
                                                                               \
    double_map.__size = m->__size * 2;                                         \
    m_bool_vec_resize(&double_map.is_idx_open, double_map.__size);             \
    map_access(Ta, Tb, resize)(&double_map.map, double_map.__size);            \
    m_bool_vec_map(&double_map.is_idx_open, m_bool_set_to_true);               \
                                                                               \
    for (int64_t slot_idx = 0; slot_idx < m->is_idx_open.length; slot_idx++) { \
      if (!*m_bool_vec_at(&m->is_idx_open, slot_idx)) {                        \
        map_item_t(Ta, Tb) *it = map_access(Ta, Tb, at)(&m->map, slot_idx);    \
        ffname(Ta, Tb, map_put)(&double_map, &it->key, &it->value);            \
      }                                                                        \
    }                                                                          \
                                                                               \
    /* Overwrite the memory at address m with the local container memory */    \
    ffname(Ta, Tb, map_free)(m);                                               \
    memmove(m, &double_map, sizeof(double_map));                               \
  }                                                                            \
                                                                               \
  static ret(int64_t) ffname(Ta, Tb, key_loc)(map_t(Ta, Tb) * m, Ta * key) {   \
    ffname(Ta, Tb, map_assert_init)(m);                                        \
    uint64_t start_idx, idx;                                                   \
                                                                               \
    start_idx = ffname(Ta, Tb, hash)(key) % m->map.length;                     \
                                                                               \
    /* Probe forward */                                                        \
    for (idx = start_idx; idx < (uint64_t)m->is_idx_open.length; idx++) {      \
      if (*m_bool_vec_at(&m->is_idx_open, idx)) continue;                      \
      map_item_t(Ta, Tb) *it = map_access(Ta, Tb, at)(&m->map, idx);           \
      if (memcmp(&it->key, key, sizeof(*key)) == 0) return idx;                \
    }                                                                          \
                                                                               \
    /* Probe failed, go to zero and probe until start_idx. */                  \
    for (idx = 0; idx < start_idx; idx++) {                                    \
      if (*m_bool_vec_at(&m->is_idx_open, idx)) continue;                      \
      map_item_t(Ta, Tb) *it = map_access(Ta, Tb, at)(&m->map, idx);           \
      if (memcmp(&it->key, key, sizeof(*key)) == 0) return idx;                \
    }                                                                          \
                                                                               \
    return -1;                                                                 \
  }

uint64_t hash_bytes(void *ptr, size_t size) {
  /* djb2 by Dan Bernstein
   * http://www.cse.yorku.ca/~oz/hash.html */
  uint64_t       hash = 5381;
  unsigned char *b_ptr = (unsigned char *)ptr;

  for (size_t i = 0; i < size; i++) {
    hash = ((hash << 5) + hash) + b_ptr[i];
  }
  return hash;
}

#endif
