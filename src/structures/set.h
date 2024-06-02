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

#include <stdbool.h>
#include <stdint.h>

#include "funcdef.h"
#include "map.h"

#define set_t(T) T##_set_t

#define SET_GEN_H(T)                                                           \
  /*-------------------------------------------------------                    \
   * Define Datastructures\                                                    \
   *-------------------------------------------------------*/                  \
  typedef struct set_t(T) set_t(T);                                            \
                                                                               \
  /*-------------------------------------------------------                    \
   * A set can be simulated with a (T, bool) set                               \
   *-------------------------------------------------------*/                  \
  MAP_GEN_H(T, bool);                                                          \
                                                                               \
  /*-------------------------------------------------------                    \
   * Container Operations                                                      \
   *-------------------------------------------------------*/                  \
  set_t(T) * fname(T, set_init)(set_t(T) * s);                                 \
  set_t(T) * fname(T, set_heap_init)(set_t(T) * s);                            \
  void    fname(T, set_free)(set_t(T) * s);                                    \
  void    fname(T, set_heap_free)(set_t(T) * s);                               \
  int64_t fname(T, set_count)(set_t(T) * s);                                   \
  /*-------------------------------------------------------                    \
   * Element Operations                                                        \
   *-------------------------------------------------------*/                  \
  bool fname(T, set_has)(set_t(T) * s, T * el);                                \
  bool fname(T, set_place)(set_t(T) * s, T * el);                              \
  bool fname(T, set_delete)(set_t(T) * s, T * el);

SET_GEN_H(int);

#endif