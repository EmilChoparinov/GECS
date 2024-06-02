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
#include "gecs_codes.h"

#define vector_t(T) T##_vec_t

#define VECTOR_GEN_H(T)                                                        \
  /*-------------------------------------------------------                    \
   * Define Datastructures                                                     \
   *-------------------------------------------------------*/                  \
  typedef struct vector_t(T) vector_t(T);                                      \
                                                                               \
  struct vector_t(T) {                                                         \
    int64_t length, __top, __size;                                             \
    T      *element_head;                                                      \
  };                                                                           \
                                                                               \
  /*-------------------------------------------------------                    \
   * Define Comparator Types                                                   \
   *-------------------------------------------------------*/                  \
  typedef bool (*pred_f(T))(T * a);                                            \
  typedef T (*unary_f(T))(T);                                                  \
  typedef T (*binary_f(T))(T, T * b);                                          \
                                                                               \
  /*-------------------------------------------------------                    \
   * CONTAINER OPERATIONS                                                      \
   *-------------------------------------------------------*/                  \
  fdecl(vector_t(T) *, T, vec_init, (vector_t(T) * v));                        \
  fdecl(retcode, T, vec_free, (vector_t(T) * v));                              \
  fdecl(vector_t(T) *, T, vec_heap_init, (void));                              \
  fdecl(void, T, vec_heap_free, (vector_t(T) * v));                            \
  fdecl(retcode, T, vec_resize, (vector_t(T) * v, int64_t size));              \
  fdecl(retcode, T, vec_copy, (vector_t(T) * dest, vector_t(T) * src));        \
  fdecl(retcode, T, vec_clear, (vector_t(T) * v));                             \
                                                                               \
  /*------------------------------------------------------- \                  \
   * Element Operations                                                        \
   *-------------------------------------------------------*/                  \
  fdecl(T *, T, vec_at, (vector_t(T) * v, int64_t i));                         \
  fdecl(retcode, T, vec_put, (vector_t(T) * v, int64_t i, T * element));       \
  fdecl(bool, T, vec_has, (vector_t(T) * v, T * element));                     \
  fdecl(retcode, T, vec_push, (vector_t(T) * v, T * element));                 \
  fdecl(void, T, vec_pop, (vector_t(T) * v));                                  \
  fdecl(T *, T, vec_top, (vector_t(T) * v));                                   \
                                                                               \
  /*-------------------------------------------------------                    \
   * Functional Operations                                                     \
   *-------------------------------------------------------*/                  \
  fdecl(int64_t, T, vec_count_if, (vector_t(T) * v, pred_f(T) f_pred));        \
  fdecl(vector_t(T) *, T, vec_filter, (vector_t(T) * v, pred_f(T) f_pred));    \
  fdecl(vector_t(T) *, T, vec_map, (vector_t(T) * v, unary_f(T) f_unary));     \
  fdecl(T, T, vec_foldl, (vector_t(T) * v, binary_f(T) f_binary, T start));

#define VECTOR_GEN_C(T)                                                        \
  static void fname(T, assert_init_checks)(vector_t(T) * v);                   \
  static void fname(T, bound_assert_checks)(vector_t(T) * v, int64_t i);       \
                                                                               \
  ret(vector_t(T) *) fname(T, vec_init)(vector_t(T) * v) {                     \
    v->length = 0;                                                             \
    v->__size = 1;                                                             \
    v->__top = 0;                                                              \
    v->element_head = calloc(v->__size, sizeof(T));                            \
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
    void *new_addr = calloc(v->__size * sizeof(T), 1);                         \
    assert(new_addr);                                                          \
    memcpy(new_addr, v->element_head, old_size * sizeof(T));                   \
    free(v->element_head);                                                     \
    v->element_head = new_addr;                                                \
    return R_OKAY;                                                             \
  }                                                                            \
                                                                               \
  ret(T *) fname(T, vec_at)(vector_t(T) * v, int64_t i) {                      \
    fname(T, bound_assert_checks)(v, i);                                       \
    return &v->element_head[i];                                                \
  }                                                                            \
                                                                               \
  ret(retcode) fname(T, vec_put)(vector_t(T) * v, int64_t i, T * element) {    \
    fname(T, bound_assert_checks)(v, i);                                       \
    assert(element);                                                           \
                                                                               \
    memmove((char *)v->element_head + i * sizeof(T), element, sizeof(T));      \
    /*v->element_head[i] = *element;*/                                         \
    return R_OKAY;                                                             \
  }                                                                            \
                                                                               \
  ret(bool) fname(T, vec_has)(vector_t(T) * v, T * element) {                  \
    fname(T, assert_init_checks)(v);                                           \
    assert(element);                                                           \
                                                                               \
    for (int64_t i = 0; i < v->length; i++)                                    \
      if (memcmp(element, &v->element_head[i], sizeof(T)) == 0) return true;   \
    return false;                                                              \
  }                                                                            \
                                                                               \
  ret(retcode) fname(T, vec_push)(vector_t(T) * v, T * element) {              \
    fname(T, assert_init_checks)(v);                                           \
    assert(element);                                                           \
    bool ret = fname(T, vec_resize)(v, v->length) == R_OKAY;                   \
    assert(ret);                                                               \
    memmove(&v->element_head[v->__top], element, sizeof(T));                   \
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
    return &v->element_head[v->__top - 1];                                     \
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
    dest->element_head = malloc(src->__size * sizeof(T));                      \
    memmove(dest->element_head, src->element_head, src->__size * sizeof(T));   \
    return R_OKAY;                                                             \
  }                                                                            \
                                                                               \
  ret(retcode) fname(T, vec_clear)(vector_t(T) * v) {                          \
    fname(T, assert_init_checks)(v);                                           \
    v->length = 0;                                                             \
    v->__top = 0;                                                              \
    return R_OKAY;                                                             \
  }                                                                            \
                                                                               \
  ret(int64_t) fname(T, vec_count_if)(vector_t(T) * v, pred_f(T) f_pred) {     \
    fname(T, assert_init_checks)(v);                                           \
    int64_t counter = 0;                                                       \
    for (int64_t i = 0; i < v->length; i++)                                    \
      if (f_pred(&v->element_head[i])) counter++;                              \
    return counter;                                                            \
  }                                                                            \
                                                                               \
  ret(vector_t(T) *) fname(T, vec_filter)(vector_t(T) * v, pred_f(T) f_pred) { \
    fname(T, assert_init_checks)(v);                                           \
                                                                               \
    vector_t(T) filter;                                                        \
    fname(T, vec_init)(&filter);                                               \
                                                                               \
    for (int64_t i = 0; i < v->length; i++)                                    \
      if (f_pred(&v->element_head[i]))                                         \
        fname(T, vec_push)(&filter, &v->element_head[i]);                      \
                                                                               \
    /* Free the internals of the *v vector and copy over the local one. */     \
    fname(T, vec_free)(v);                                                     \
    memmove(v, &filter, sizeof(*v));                                           \
    return v;                                                                  \
  }                                                                            \
                                                                               \
  ret(vector_t(T) *) fname(T, vec_map)(vector_t(T) * v, unary_f(T) f_unary) {  \
    fname(T, assert_init_checks)(v);                                           \
                                                                               \
    for (int64_t i = 0; i < v->length; i++)                                    \
      v->element_head[i] = f_unary(v->element_head[i]);                        \
                                                                               \
    return v;                                                                  \
  }                                                                            \
                                                                               \
  ret(T) fname(T, vec_foldl)(vector_t(T) * v, binary_f(T) f_binary, T start) { \
    fname(T, assert_init_checks)(v);                                           \
                                                                               \
    T result = start;                                                          \
    for (int64_t i = 0; i < v->length; i++)                                    \
      result = f_binary(result, &v->element_head[i]);                          \
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
  }

#endif
