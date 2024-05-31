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

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "gecs_codes.h"

#define vector_t(T)    vec_##T##_t
#define fname(T, name) vec_##T##_##name
#define pred_f(T)      vec_##T##_predicate
#define unary_f(T)     vec_##T##_unary
#define binary_f(T)    vec_##T##_binary

#define fdecl(ret, T, name, body) ret fname(T, name) body

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
  fdecl(vector_t(T) *, T, init, (vector_t(T) * v));                            \
  fdecl(gecs_retcode, T, free, (vector_t(T) * v));                             \
  fdecl(vector_t(T) *, T, heap_init, (void));                                  \
  fdecl(void, T, heap_free, (vector_t(T) * v));                                \
  fdecl(gecs_retcode, T, resize, (vector_t(T) * v, int64_t size));             \
                                                                               \
  /*------------------------------------------------------- \                  \
   * Element Operations                                                        \
   *-------------------------------------------------------*/                  \
  fdecl(T *, T, at, (vector_t(T) * v, int64_t i));                             \
  fdecl(gecs_retcode, T, put, (vector_t(T) * v, int64_t i, T * element));      \
  fdecl(bool, T, has, (vector_t(T) * v, T * element));                         \
  fdecl(gecs_retcode, T, push, (vector_t(T) * v, T * element));                \
  fdecl(void, T, pop, (vector_t(T) * v));                                      \
  fdecl(T *, T, top, (vector_t(T) * v));                                       \
  fdecl(gecs_retcode, T, copy, (vector_t(T) * dest, vector_t(T) * src));       \
  fdecl(gecs_retcode, T, clear, (vector_t(T) * v));                            \
                                                                               \
  /*-------------------------------------------------------                    \
   * Functional Operations                                                     \
   *-------------------------------------------------------*/                  \
  fdecl(int64_t, T, count_if, (vector_t(T) * v, pred_f(T) f_pred));            \
  fdecl(vector_t(T) *, T, filter, (vector_t(T) * v, pred_f(T) f_pred));        \
  fdecl(vector_t(T) *, T, map, (vector_t(T) * v, unary_f(T) f_unary));         \
  fdecl(T, T, foldl, (vector_t(T) * v, binary_f(T) f_binary, T start));

typedef struct x_t x_t;
struct x_t {
  int a, b, c;
};
VECTOR_GEN_H(int)
VECTOR_GEN_H(char)
VECTOR_GEN_H(x_t)

#define VECTOR_GEN_C(T)                                                        \
  static void vec_##T##_assert_init_checks(vector_t(T) * v);                   \
  static void vec_##T##_bound_assert_checks(vector_t(T) * v, int64_t i);       \
                                                                               \
  fdecl(                                                                       \
      vector_t(T) *, T, init, (vector_t(T) * v) {                              \
        v->length = 0;                                                         \
        v->__size = 1;                                                         \
        v->__top = 0;                                                          \
        v->element_head = malloc(v->__size * sizeof(T));                       \
        assert(v->element_head);                                               \
        return v;                                                              \
      });                                                                      \
                                                                               \
  fdecl(                                                                       \
      gecs_retcode, T, free, (vector_t(T) * v) {                               \
        assert(v);                                                             \
        assert(v->element_head != NULL);                                       \
                                                                               \
        free(v->element_head);                                                 \
        return GECS_OKAY;                                                      \
      });                                                                      \
                                                                               \
  fdecl(                                                                       \
      vector_t(T) *, T, heap_init, (void) {                                    \
        vector_t(T) *v = malloc(sizeof(*v));                                   \
        assert(v);                                                             \
                                                                               \
        fname(T, init)(v);                                                     \
        return v;                                                              \
      });                                                                      \
                                                                               \
  fdecl(                                                                       \
      void, T, heap_free, (vector_t(T) * v) {                                  \
        fname(T, free)(v);                                                     \
        free(v);                                                               \
      });                                                                      \
                                                                               \
  fdecl(                                                                       \
      gecs_retcode, T, resize, (vector_t(T) * v, int64_t size) {               \
        if (size < v->__size) return GECS_OKAY;                                \
        while (size >= v->__size) v->__size *= 2;                              \
        v->element_head = realloc(v->element_head, v->__size * sizeof(T));     \
        v->length = size;                                                      \
        return GECS_OKAY;                                                      \
      });                                                                      \
                                                                               \
  fdecl(                                                                       \
      T *, T, at, (vector_t(T) * v, int64_t i) {                               \
        vec_##T##_bound_assert_checks(v, i);                                   \
        return &v->element_head[i];                                            \
      });                                                                      \
                                                                               \
  fdecl(                                                                       \
      gecs_retcode, T, put, (vector_t(T) * v, int64_t i, T * element) {        \
        vec_##T##_bound_assert_checks(v, i);                                   \
        assert(element);                                                       \
                                                                               \
        v->element_head[i] = *element;                                         \
        return GECS_OKAY;                                                      \
      });                                                                      \
                                                                               \
  fdecl(                                                                       \
      bool, T, has, (vector_t(T) * v, T * element) {                           \
        vec_##T##_assert_init_checks(v);                                       \
        assert(element);                                                       \
                                                                               \
        for (int64_t i = 0; i < v->length; i++)                                \
          if (memcmp(element, &v->element_head[i], sizeof(T)) == 0)            \
            return true;                                                       \
        return false;                                                          \
      });                                                                      \
                                                                               \
  fdecl(                                                                       \
      gecs_retcode, T, push, (vector_t(T) * v, T * element) {                  \
        vec_##T##_assert_init_checks(v);                                       \
        assert(element);                                                       \
        bool ret = fname(T, resize)(v, v->length) == GECS_OKAY;                \
        assert(ret);                                                           \
        memmove(&v->element_head[v->__top], element, sizeof(T));               \
        v->__top++;                                                            \
        if (v->__top >= v->length) v->length = v->__top;                       \
        return ret;                                                            \
      });                                                                      \
                                                                               \
  fdecl(                                                                       \
      void, T, pop, (vector_t(T) * v) {                                        \
        vec_##T##_assert_init_checks(v);                                       \
        assert(v->__top);                                                      \
        if (v->__top == v->length) v->length--;                                \
        v->__top--;                                                            \
      });                                                                      \
                                                                               \
  fdecl(                                                                       \
      T *, T, top, (vector_t(T) * v) {                                         \
        vec_##T##_assert_init_checks(v);                                       \
        return &v->element_head[v->__top - 1];                                 \
      });                                                                      \
                                                                               \
  fdecl(                                                                       \
      gecs_retcode, T, copy, (vector_t(T) * dest, vector_t(T) * src) {         \
        vec_##T##_assert_init_checks(dest);                                    \
        vec_##T##_assert_init_checks(src);                                     \
                                                                               \
        fname(T, free)(dest);                                                  \
        /* Copy metadata first */                                              \
        memmove(dest, src, sizeof(*src));                                      \
        if (dest->length == 0) return GECS_OKAY;                               \
                                                                               \
        /* Copy array into new dest*/                                          \
        dest->element_head = malloc(src->__size * sizeof(T));                  \
        memmove(dest->element_head, src->element_head,                         \
                src->__size * sizeof(T));                                      \
        return GECS_OKAY;                                                      \
      });                                                                      \
                                                                               \
  fdecl(                                                                       \
      gecs_retcode, T, clear, (vector_t(T) * v) {                              \
        vec_##T##_assert_init_checks(v);                                       \
        v->length = 0;                                                         \
        v->__top = 0;                                                          \
        return GECS_OKAY;                                                      \
      });                                                                      \
                                                                               \
  fdecl(                                                                       \
      int64_t, T, count_if, (vector_t(T) * v, pred_f(T) f_pred) {              \
        vec_##T##_assert_init_checks(v);                                       \
        int64_t counter = 0;                                                   \
        for (int64_t i = 0; i < v->length; i++)                                \
          if (f_pred(&v->element_head[i])) counter++;                          \
        return counter;                                                        \
      });                                                                      \
                                                                               \
  fdecl(                                                                       \
      vector_t(T) *, T, filter, (vector_t(T) * v, pred_f(T) f_pred) {          \
        vec_##T##_assert_init_checks(v);                                       \
                                                                               \
        vector_t(T) filter;                                                    \
        fname(T, init)(&filter);                                               \
                                                                               \
        for (int64_t i = 0; i < v->length; i++)                                \
          if (f_pred(&v->element_head[i]))                                     \
            fname(T, push)(&filter, &v->element_head[i]);                      \
                                                                               \
        /* Free the internals of the *v vector and copy over the local one. */ \
        fname(T, free)(v);                                                     \
        memmove(v, &filter, sizeof(*v));                                       \
        return v;                                                              \
      });                                                                      \
                                                                               \
  fdecl(                                                                       \
      vector_t(T) *, T, map, (vector_t(T) * v, unary_f(T) f_unary) {           \
        vec_##T##_assert_init_checks(v);                                       \
                                                                               \
        for (int64_t i = 0; i < v->length; i++)                                \
          v->element_head[i] = f_unary(v->element_head[i]);                    \
                                                                               \
        return v;                                                              \
      });                                                                      \
                                                                               \
  fdecl(                                                                       \
      T, T, foldl, (vector_t(T) * v, binary_f(T) f_binary, T start) {          \
        vec_##T##_assert_init_checks(v);                                       \
                                                                               \
        T result = start;                                                      \
        for (int64_t i = 0; i < v->length; i++)                                \
          result = f_binary(result, &v->element_head[i]);                      \
                                                                               \
        return result;                                                         \
      });                                                                      \
                                                                               \
  static void vec_##T##_assert_init_checks(vector_t(T) * v) {                  \
    assert(v);                                                                 \
    assert(v->element_head);                                                   \
  }                                                                            \
                                                                               \
  static void vec_##T##_bound_assert_checks(vector_t(T) * v, int64_t i) {      \
    vec_##T##_assert_init_checks(v);                                           \
    assert(i >= 0 && i < v->length);                                           \
  }

#endif