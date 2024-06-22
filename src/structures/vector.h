/* =========================================================================
    Author: E.D Choparinov, Amsterdam
    Related Files: vector.h
    Created On: May 31 2024
    Purpose:
        The purpose of this file is to provide a generate for generic
        arraylike structures. With some functional support!
========================================================================= */

#ifndef __HEADER_VECTOR_H__
#define __HEADER_VECTOR_H__

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "funcdef.h"
#include "logger.h"
#include "retcodes.h"
#include "str_utils.h"

#define vector_t(T) T##_vec_t

// This internal type does **not** matter! We only want the type info.
typedef void *any;

#define of_any(X) (void *)(X)

#define VECTOR_GEN_H(T)                                                        \
  /*-------------------------------------------------------                    \
   * Define Datastructures                                                     \
   *-------------------------------------------------------*/                  \
  typedef struct vector_t(T) vector_t(T);                                      \
                                                                               \
  struct vector_t(T) {                                                         \
    int64_t length, __top, __size;                                             \
    int64_t __el_size;                                                         \
    T      *element_head;                                                      \
  };                                                                           \
                                                                               \
  /*-------------------------------------------------------                    \
   * Define Comparator Types                                                   \
   *-------------------------------------------------------*/                  \
  typedef bool (*pred_f(T))(T * a, void *arg);                                 \
  typedef T (*unary_f(T))(T, void *arg);                                       \
  typedef T (*binary_f(T))(T, T * b, void *arg);                               \
  typedef bool (*compare_f(T))(T * a, T * b, void *arg);                       \
                                                                               \
  /*-------------------------------------------------------                    \
   * CONTAINER OPERATIONS                                                      \
   *-------------------------------------------------------*/                  \
  fdecl(vector_t(T) *, T, vec_init, (vector_t(T) * v));                        \
  fdecl(retcode, T, vec_free, (vector_t(T) * v));                              \
  fdecl(vector_t(T) *, T, vec_heap_init, (void));                              \
  fdecl(void, T, vec_heap_free, (vector_t(T) * v));                            \
                                                                               \
  fdecl(retcode, T, vec_resize, (vector_t(T) * v, int64_t size));              \
  fdecl(retcode, T, vec_copy, (vector_t(T) * dest, vector_t(T) * src));        \
  fdecl(retcode, T, vec_clear, (vector_t(T) * v));                             \
  fdecl(retcode, T, vec_sort, (vector_t(T) * v, compare_f(T) cmp, void *arg)); \
                                                                               \
  /*------------------------------------------------------- \                  \
   * Element Operations                                                        \
   *-------------------------------------------------------*/                  \
  fdecl(T *, T, vec_at, (vector_t(T) * v, int64_t i));                         \
  fdecl(retcode, T, vec_put, (vector_t(T) * v, int64_t i, T * element));       \
  fdecl(retcode, T, vec_delete_at, (vector_t(T) * v, int64_t idx));            \
  fdecl(retcode, T, vec_delete, (vector_t(T) * v, T * el));                    \
  fdecl(bool, T, vec_has, (vector_t(T) * v, T * element));                     \
  fdecl(int64_t, T, vec_find, (vector_t(T) * v, T * element));                 \
  fdecl(retcode, T, vec_push, (vector_t(T) * v, T * element));                 \
  fdecl(void, T, vec_pop, (vector_t(T) * v));                                  \
  fdecl(T *, T, vec_top, (vector_t(T) * v));                                   \
  fdecl(retcode, T, vec_swap, (vector_t(T) * v, int64_t a, int64_t b));        \
                                                                               \
  /*-------------------------------------------------------                    \
   * Functional Operations                                                     \
   *-------------------------------------------------------*/                  \
  fdecl(int64_t, T, vec_count_if,                                              \
        (vector_t(T) * v, pred_f(T) f_pred, void *arg));                       \
  fdecl(vector_t(T) *, T, vec_filter,                                          \
        (vector_t(T) * v, pred_f(T) f_pred, void *arg));                       \
  fdecl(void, T, vec_foreach, (vector_t(T) * v, pred_f(T) f_pred, void *arg)); \
  fdecl(vector_t(T) *, T, vec_map,                                             \
        (vector_t(T) * v, unary_f(T) f_unary, void *arg));                     \
  fdecl(T, T, vec_foldl,                                                       \
        (vector_t(T) * v, binary_f(T) f_binary, T start, void *arg));

#define VECTOR_GEN_C(T)                                                        \
  static void fname(T, assert_init_checks)(vector_t(T) * v);                   \
  static void fname(T, bound_assert_checks)(vector_t(T) * v, int64_t i);       \
  static ret(vector_t(T) *) fname(T, vec_construct)(vector_t(T) * v);          \
  static ret(T *) fname(T, _d_vec_at)(vector_t(T) * v, int64_t i);             \
                                                                               \
  ret(vector_t(T) *) fname(T, vec_construct)(vector_t(T) * v) {                \
    v->length = 0;                                                             \
    v->__size = 1;                                                             \
    v->__top = 0;                                                              \
    return v;                                                                  \
  }                                                                            \
                                                                               \
  ret(vector_t(T) *) fname(T, vec_init)(vector_t(T) * v) {                     \
    fname(T, vec_construct)(v);                                                \
    v->__el_size = sizeof(T);                                                  \
    v->element_head = calloc(v->__size, v->__el_size);                         \
    assert(v->element_head);                                                   \
    return v;                                                                  \
  }                                                                            \
                                                                               \
  ret(retcode) fname(T, vec_free)(vector_t(T) * v) {                           \
    assert(v);                                                                 \
    assert(v->element_head != NULL);                                           \
                                                                               \
    free(v->element_head);                                                     \
    return R_OKAY;                                                             \
  }                                                                            \
                                                                               \
  ret(vector_t(T) *) fname(T, vec_heap_init)(void) {                           \
    vector_t(T) *v = malloc(sizeof(*v));                                       \
    assert(v);                                                                 \
                                                                               \
    fname(T, vec_init)(v);                                                     \
    return v;                                                                  \
  }                                                                            \
                                                                               \
  ret(void) fname(T, vec_heap_free)(vector_t(T) * v) {                         \
    fname(T, vec_free)(v);                                                     \
    free(v);                                                                   \
  }                                                                            \
                                                                               \
  ret(retcode) fname(T, vec_resize)(vector_t(T) * v, int64_t size) {           \
    fname(T, assert_init_checks)(v);                                           \
    v->length = size;                                                          \
    if (size < v->__size) return R_OKAY;                                       \
    int64_t old_size = v->__size;                                              \
    while (size >= v->__size) v->__size *= 2;                                  \
    void *new_addr = calloc(v->__size * v->__el_size, 1);                      \
    assert(new_addr);                                                          \
    memcpy(new_addr, v->element_head, old_size * v->__el_size);                \
    free(v->element_head);                                                     \
    v->element_head = new_addr;                                                \
    return R_OKAY;                                                             \
  }                                                                            \
                                                                               \
  ret(T *) fname(T, vec_at)(vector_t(T) * v, int64_t i) {                      \
    fname(T, bound_assert_checks)(v, i);                                       \
    return fname(T, _d_vec_at)(v, i);                                          \
  }                                                                            \
                                                                               \
  ret(retcode) fname(T, vec_put)(vector_t(T) * v, int64_t i, T * element) {    \
    fname(T, bound_assert_checks)(v, i);                                       \
    assert(element);                                                           \
                                                                               \
    memmove((char *)v->element_head + i * v->__el_size, element,               \
            v->__el_size);                                                     \
    return R_OKAY;                                                             \
  }                                                                            \
                                                                               \
  ret(retcode) fname(T, vec_delete_at)(vector_t(T) * v, int64_t idx) {         \
    fname(T, assert_init_checks)(v);                                           \
    if (v->length - 1 == idx) {                                                \
      fname(T, vec_pop)(v);                                                    \
      return R_OKAY;                                                           \
    }                                                                          \
    void *loc = (char *)v->element_head + idx * v->__el_size;                  \
    void *loc_next = (char *)v->element_head + (idx + 1) * v->__el_size;       \
    memmove(loc, loc_next, v->length * v->__el_size - idx - v->__el_size);     \
    v->length--;                                                               \
    v->__top--;                                                                \
    return R_OKAY;                                                             \
  }                                                                            \
                                                                               \
  ret(retcode) fname(T, vec_delete)(vector_t(T) * v, T * el) {                 \
    fname(T, assert_init_checks)(v);                                           \
    int64_t idx = fname(T, vec_find)(v, el);                                   \
    assert(idx != -1);                                                         \
    if (idx == v->length - 1) {                                                \
      fname(T, vec_pop)(v);                                                    \
      return R_OKAY;                                                           \
    }                                                                          \
    return fname(T, vec_delete_at)(v, idx);                                    \
  }                                                                            \
                                                                               \
  ret(bool) fname(T, vec_has)(vector_t(T) * v, T * element) {                  \
    fname(T, assert_init_checks)(v);                                           \
    assert(element);                                                           \
    return fname(T, vec_find)(v, element) != -1;                               \
  }                                                                            \
                                                                               \
  ret(int64_t) fname(T, vec_find)(vector_t(T) * v, T * element) {              \
    fname(T, assert_init_checks)(v);                                           \
    assert(element);                                                           \
                                                                               \
    for (int64_t i = 0; i < v->length; i++)                                    \
      if (memcmp(element, fname(T, vec_at)(v, i), v->__el_size) == 0)          \
        return i;                                                              \
    return -1;                                                                 \
  }                                                                            \
                                                                               \
  ret(retcode) fname(T, vec_push)(vector_t(T) * v, T * element) {              \
    fname(T, assert_init_checks)(v);                                           \
    assert(element);                                                           \
    bool ret = fname(T, vec_resize)(v, v->length) == R_OKAY;                   \
    assert(ret);                                                               \
    memmove(fname(T, _d_vec_at)(v, v->__top), element, v->__el_size);          \
    v->__top++;                                                                \
    if (v->__top >= v->length) v->length = v->__top;                           \
    return ret;                                                                \
  }                                                                            \
                                                                               \
  ret(void) fname(T, vec_pop)(vector_t(T) * v) {                               \
    fname(T, assert_init_checks)(v);                                           \
    assert(v->__top);                                                          \
    if (v->__top == v->length) v->length--;                                    \
    v->__top--;                                                                \
  }                                                                            \
                                                                               \
  ret(T *) fname(T, vec_top)(vector_t(T) * v) {                                \
    fname(T, assert_init_checks)(v);                                           \
    return fname(T, vec_at)(v, v->__top - 1);                                  \
  }                                                                            \
  ret(retcode) fname(T, vec_swap)(vector_t(T) * v, int64_t a, int64_t b) {     \
    T *a_data = fname(T, vec_at)(v, a);                                        \
    T *b_data = fname(T, vec_at)(v, b);                                        \
    memswap(a_data, b_data, sizeof(T));                                        \
    return R_OKAY;                                                             \
  }                                                                            \
                                                                               \
  ret(retcode) fname(T, vec_copy)(vector_t(T) * dest, vector_t(T) * src) {     \
    fname(T, assert_init_checks)(dest);                                        \
    fname(T, assert_init_checks)(src);                                         \
                                                                               \
    fname(T, vec_free)(dest);                                                  \
    /* Copy metadata first */                                                  \
    memmove(dest, src, sizeof(*src));                                          \
    if (dest->length == 0) return R_OKAY;                                      \
                                                                               \
    /* Copy array into new dest*/                                              \
    dest->element_head = malloc(src->__size * src->__el_size);                 \
    memmove(dest->element_head, src->element_head,                             \
            src->__size * src->__el_size);                                     \
    return R_OKAY;                                                             \
  }                                                                            \
                                                                               \
  ret(retcode) fname(T, vec_clear)(vector_t(T) * v) {                          \
    fname(T, assert_init_checks)(v);                                           \
    v->length = 0;                                                             \
    v->__top = 0;                                                              \
    return R_OKAY;                                                             \
  }                                                                            \
  ret(retcode)                                                                 \
      fname(T, vec_sort)(vector_t(T) * v, compare_f(T) cmp, void *arg) {       \
    fname(T, assert_init_checks)(v);                                           \
                                                                               \
    int64_t i, j;                                                              \
    for (i = 0; i < v->length; i++) {                                          \
      for (j = 0; j < v->length - i - 1; j++) {                                \
        if (!cmp(fname(T, vec_at)(v, j), fname(T, vec_at)(v, j + 1), arg))     \
          fname(T, vec_swap)(v, j, j + 1);                                     \
      }                                                                        \
    }                                                                          \
    return R_OKAY;                                                             \
  }                                                                            \
                                                                               \
  ret(int64_t)                                                                 \
      fname(T, vec_count_if)(vector_t(T) * v, pred_f(T) f_pred, void *arg) {   \
    fname(T, assert_init_checks)(v);                                           \
    int64_t counter = 0;                                                       \
    for (int64_t i = 0; i < v->length; i++)                                    \
      if (f_pred(&v->element_head[i], arg)) counter++;                         \
    return counter;                                                            \
  }                                                                            \
                                                                               \
  ret(vector_t(T) *)                                                           \
      fname(T, vec_filter)(vector_t(T) * v, pred_f(T) f_pred, void *arg) {     \
    fname(T, assert_init_checks)(v);                                           \
                                                                               \
    vector_t(T) filter;                                                        \
    fname(T, vec_init)(&filter);                                               \
                                                                               \
    for (int64_t i = 0; i < v->length; i++)                                    \
      if (f_pred(&v->element_head[i], arg))                                    \
        fname(T, vec_push)(&filter, &v->element_head[i]);                      \
                                                                               \
    /* Free the internals of the *v vector and copy over the local one. */     \
    fname(T, vec_free)(v);                                                     \
    memmove(v, &filter, sizeof(*v));                                           \
    return v;                                                                  \
  }                                                                            \
                                                                               \
  ret(void)                                                                    \
      fname(T, vec_foreach)(vector_t(T) * v, pred_f(T) f_pred, void *arg) {    \
    fname(T, assert_init_checks)(v);                                           \
    for (int64_t i = 0; i < v->length; i++) f_pred(&v->element_head[i], arg);  \
  }                                                                            \
                                                                               \
  ret(vector_t(T) *)                                                           \
      fname(T, vec_map)(vector_t(T) * v, unary_f(T) f_unary, void *arg) {      \
    fname(T, assert_init_checks)(v);                                           \
                                                                               \
    for (int64_t i = 0; i < v->length; i++)                                    \
      v->element_head[i] = f_unary(v->element_head[i], arg);                   \
                                                                               \
    return v;                                                                  \
  }                                                                            \
                                                                               \
  ret(T) fname(T, vec_foldl)(vector_t(T) * v, binary_f(T) f_binary, T start,   \
                             void *arg) {                                      \
    fname(T, assert_init_checks)(v);                                           \
                                                                               \
    T result = start;                                                          \
    for (int64_t i = 0; i < v->length; i++)                                    \
      result = f_binary(result, &v->element_head[i], arg);                     \
                                                                               \
    return result;                                                             \
  }                                                                            \
                                                                               \
  static ret(void) fname(T, assert_init_checks)(vector_t(T) * v) {             \
    assert(v);                                                                 \
    assert(v->element_head);                                                   \
  }                                                                            \
                                                                               \
  static ret(void) fname(T, bound_assert_checks)(vector_t(T) * v, int64_t i) { \
    fname(T, assert_init_checks)(v);                                           \
    assert(i >= 0 && i < v->length);                                           \
  }                                                                            \
  static ret(T *) fname(T, _d_vec_at)(vector_t(T) * v, int64_t i) {            \
    return (void *)(v->element_head) + i * v->__el_size;                       \
  }

VECTOR_GEN_H(any);

#define vec_unknown_type_free      any_vec_free
#define vec_unknown_type_heap_free any_vec_heap_free

any_vec_t *vec_unknown_type_init(any_vec_t *v, size_t el_size);
any_vec_t *vec_unknown_type_heap_init(size_t el_size);

/*-------------------------------------------------------
 * Utility Functions
 *-------------------------------------------------------*/
#define vectors_intersect(a, b) __vectors_intersect(of_any(a), of_any(b))

bool __vectors_intersect(any_vec_t *a, any_vec_t *b);

#endif
