/* =========================================================================
    Author: E.D Choparinov, Amsterdam
    Related Files: set.h
    Created On: June 02 2024
    Purpose:
        The purpose of this file is to provide macro generation code for
        efficient set type datastructures.
========================================================================= */

#ifndef __HEADER_SET_H__
#define __HEADER_SET_H__

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "funcdef.h"
#include "gecs_codes.h"
#include "map.h"

#define set_t(T)      T##_set_t
#define set_access(T) wrap_set_##T##_char_map_t
#define set_slot_access(T)
#define set_func_access(T, func) ffname(wrap_set_##T, char, func)

#define SET_GEN_H(T)                                                           \
  /*-------------------------------------------------------                    \
   * Define Datastructures\                                                    \
   *-------------------------------------------------------*/                  \
  typedef struct set_t(T) set_t(T);                                            \
  typedef T wrap_type(T, set);                                                 \
                                                                               \
  /*-------------------------------------------------------                    \
   * A set can be simulated with a (T, bool) set                               \
   *-------------------------------------------------------*/                  \
  MAP_GEN_H(wrap_type(T, set), char);                                          \
                                                                               \
  /*-------------------------------------------------------                    \
   * Container Operations                                                      \
   *-------------------------------------------------------*/                  \
  set_t(T) * fname(T, set_init)(set_t(T) * s);                                 \
  set_t(T) * fname(T, set_heap_init)(set_t(T) * s);                            \
  void    fname(T, set_free)(set_t(T) * s);                                    \
  void    fname(T, set_heap_free)(set_t(T) * s);                               \
  void    fname(T, set_clear)(set_t(T) * s);                                   \
  int64_t fname(T, set_count)(set_t(T) * s);                                   \
  /*-------------------------------------------------------                    \
   * Element Operations                                                        \
   *-------------------------------------------------------*/                  \
  bool fname(T, set_has)(set_t(T) * s, T * el);                                \
  bool fname(T, set_place)(set_t(T) * s, T * el);                              \
  bool fname(T, set_delete)(set_t(T) * s, T * el);

#define SET_GEN_C(T)                                                           \
  /*-------------------------------------------------------                    \
   * Define Datastructures\                                                    \
   *-------------------------------------------------------*/                  \
  struct set_t(T) { set_access(T) map; };                                      \
                                                                               \
  static void fname(T, set_assert_init)(set_t(T) * s);                         \
                                                                               \
  MAP_GEN_C(wrap_type(T, set), char);                                          \
                                                                               \
  /*-------------------------------------------------------                    \
   * Container Operations                                                      \
   *-------------------------------------------------------*/                  \
  set_t(T) * fname(T, set_init)(set_t(T) * s) {                                \
    set_func_access(T, map_init)(&s->map);                                     \
    return s;                                                                  \
  }                                                                            \
  set_t(T) * fname(T, set_heap_init)(set_t(T) * s) {                           \
    s = malloc(sizeof(*s));                                                    \
    assert(s);                                                                 \
    return fname(T, set_init)(s);                                              \
  }                                                                            \
                                                                               \
  void fname(T, set_free)(set_t(T) * s) {                                      \
    fname(T, set_assert_init)(s);                                              \
    set_func_access(T, map_free)(&s->map);                                     \
  }                                                                            \
                                                                               \
  void fname(T, set_heap_free)(set_t(T) * s) {                                 \
    fname(T, set_assert_init)(s);                                              \
    fname(T, set_free)(s);                                                     \
    free(s);                                                                   \
  }                                                                            \
                                                                               \
  void fname(T, set_clear)(set_t(T) * s) {                                     \
    m_bool_vec_clear(&s->map.is_idx_open);                                     \
  }                                                                            \
                                                                               \
  bool    is_true(bool *b) { return *b; }                                      \
  int64_t fname(T, set_count)(set_t(T) * s) { return s->map.slots_in_use; }    \
  /*-------------------------------------------------------                    \
   * Element Operations                                                        \
   *-------------------------------------------------------*/                  \
  bool fname(T, set_has)(set_t(T) * s, T * el) {                               \
    return set_func_access(T, map_has)(&s->map, el);                           \
  }                                                                            \
                                                                               \
  bool fname(T, set_place)(set_t(T) * s, T * el) {                             \
    char v = 0;                                                                \
    return set_func_access(T, map_put)(&s->map, el, &v) == R_OKAY;             \
  }                                                                            \
                                                                               \
  bool fname(T, set_delete)(set_t(T) * s, T * el) {                            \
    return set_func_access(T, map_remove)(&s->map, el) == R_OKAY;              \
  }                                                                            \
                                                                               \
  static void fname(T, set_assert_init)(set_t(T) * s) {                        \
    assert(s);                                                                 \
    assert(s->map.is_idx_open.element_head);                                   \
    assert(s->map.map.element_head);                                           \
  }

#endif