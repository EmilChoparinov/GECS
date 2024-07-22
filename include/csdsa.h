/* =========================================================================
Author: E.D Choparinov, Amsterdam
Related Files: csdsa.h
Created On: June 28 2024
Purpose:
    The purpose of this library is to provide various data structures
    and algorithms for solving general computing tasks in C. The library
    leans towards being as functional as possible, so most calls to this
    library require passing in functions.

    This library also features an allocator that provides stack or random
    heap allocation modes. The user can specify either directly which
    allocator instance to use or to use a globally existing allocator.

The following are the contents and descriptions of each part of this
library:

=== Allocator ===
The allocator built into this library is called stalloc (as in for stack
allocator) and supports stack allocation and random heap allocation. The
allocator utilizes the same contiguous memory segment for both heap and
stack allocation for best performance.

Only random heap allocations may use the libraries free function. Stack
context allocations use pop() or the built in global stack pop function.

=== Data Structures ===
vec_*   : Wrapper for a contiguous segment of memory. Can be
                  used to represent a fixed or resizable array.

map_*   : Open address hashing table with resize capabilities.

set_*   : Implemented as a resizeable sparse set wrapped with
          the libraries hashing function.

stack_* : Wrapper over vec_* and constrains the vector.

queue_* : Wrapper over vec_* again and constrains the vector.

graph_* : Implemented as a map of sets. Each vertex stores a pointer to
          some address you define.

buff_*  : Buffer API implementation that takes a region of memory and allows
          for various manipulations

=== Algorithms ===
hash_bytes() : Modified version of djb2 to consume N bytes. Originally from:
                http://www.cse.yorku.ca/~oz/hash.html

bfs()        : Breadth first search over a graph
dfs()        : Depth first search over a graph
a_star()     : The classic a* graph search algorithm.
========================================================================= */
#ifndef __HEADER_CSDSA_H__
#define __HEADER_CSDSA_H__

#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*-------------------------------------------------------
 * libcsdsa Configurations
 *-------------------------------------------------------*/
// #define DISABLE_ALLOCATOR

#define TO_STACK            0
#define TO_HEAP             1
#define VECTOR_DEFAULT_SIZE 1
#define STACK_DEFAULT_SIZE  1
#define MAP_DEFAULT_SIZE    32
#define MAP_LOAD_FACTOR     0.75
#define ARENA_DEFAULT_SIZE  128
/*-------------------------------------------------------
 * Type Definitions
 *-------------------------------------------------------*/
typedef struct stalloc stalloc;
typedef struct cbuff   cbuff;

/* Internal typedefs for a functional programming style. */
typedef void (*_each)(void *el, void *args);
typedef bool (*_pred)(void *el, void *args);
typedef void (*_unary)(void *out, void *el, void *args);
typedef void (*_binary)(void *out, void *a, void *b, void *args);
typedef bool (*_compare)(void *a, void *b, void *args);

/* Macros to create FP functions. */
#define feach(name, ty, prop, body)                                            \
  void name(void *_el, void *args) {                                           \
    ty prop = *(ty *)_el;                                                      \
    body                                                                       \
  }

#define pred(name, ty, prop, body)                                             \
  bool name(void *_el, void *args) {                                           \
    ty prop = *(ty *)_el;                                                      \
    body;                                                                      \
  }

#define unary(name, ty, prop, body)                                            \
  ty name##_un(void *_el, void *args) {                                        \
    ty prop = *(ty *)_el;                                                      \
    body;                                                                      \
  }                                                                            \
  void name(void *out, void *_el, void *args) {                                \
    ty prop = name##_un(_el, args);                                            \
    memcpy(out, &prop, sizeof(ty));                                            \
  }

#define binary(name, ty, prop_a, prop_b, body)                                 \
  ty name##_bin(void *a, void *b, void *args) {                                \
    ty prop_a = *(ty *)_a;                                                     \
    ty prop_b = *(ty *)_b;                                                     \
    body;                                                                      \
  }                                                                            \
  void name(void *out, void *a, void *b, void *args) {                         \
    ty prop = name##_bin(a, b, args);                                          \
    memcpy(out, &prop, sizeof(ty));                                            \
  }

#define fold(ret_ty, name, ty, result_prop, prop_a, body)                      \
  ret_ty name##_bin(void *_res, void *_a, void *args) {                        \
    ret_ty result_prop = *(ret_ty *)_res;                                      \
    ty     prop_a = *(ty *)_a;                                                 \
    body;                                                                      \
  }                                                                            \
  void name(void *out, void *a, void *b, void *args) {                         \
    ret_ty prop = name##_bin(a, b, args);                                      \
    memcpy(out, &prop, sizeof(ret_ty));                                        \
  }

#define compare(name, ty, prop_a, prop_b, body)                                \
  bool name(void *_a, void *_b, void *args) {                                  \
    ty prop_a = *(ty *)_a;                                                     \
    ty prop_b = *(ty *)_b;                                                     \
    body;                                                                      \
  }

/* =========================================================================
   Section: Allocator
========================================================================= */
#define STALLOC_DEFAULT 8192 /* The default size to pass to stalloc_create */
#define STACK_FRAME_GUESS                                                      \
  128 /* The count of frames to generate, resize is                            \
         implemented. */

/* Initiate a global stack frame in the heap */
#define GFRAME(alloc, call)                                                    \
  {                                                                            \
    stalloc *__##alloc##_prev = get_frame_ctx();                               \
    start_frame(alloc);                                                        \
    call;                                                                      \
    end_frame(alloc);                                                          \
    set_frame_ctx(__##alloc##_prev);                                           \
  }

/* Initiate a local frame in the allocator */
#define FRAME(alloc, call)                                                     \
  start_frame(alloc);                                                          \
  call;                                                                        \
  end_frame(alloc);

void start_frame(stalloc *alloc);
void end_frame(stalloc *alloc);

/* Container Operations */
stalloc *stalloc_create(int64_t bytes);
void     stalloc_free(stalloc *alloc);

/* Stack Allocation Strategy */
#define stpusha(alloc, bytes) __stpush(alloc, bytes)
#define stpush(bytes)         __stpushframe(bytes)

void *__stpush(stalloc *alloc, int64_t bytes);
void *__stpushframe(int64_t bytes);

#define stpopa(alloc) __stpop(alloc)
#define stpop()       __stpopframe()
void __stpop(stalloc *alloc);
void __stpopframe(void);

stalloc *get_frame_ctx(void);
void     set_frame_ctx(stalloc *a);

/* Heap Allocation Strategy */
// TODO: Make into arena!
void *halloc(stalloc *alloc, int64_t bytes);
void *hrealloc(stalloc *alloc, void *ptr, int64_t bytes);
void  hfree(stalloc *alloc, void *ptr);

/* =========================================================================
  Section: Vector
========================================================================= */
typedef struct vec vec;
struct vec {
  int64_t  __top, __size, __el_size;
  int64_t  length;
  int32_t  flags;
  int32_t  cache_counter; /* Increments each resize for invalidation checks. */
  stalloc *allocator;
  void    *elements;
};

/* Container Operations */
vec  *__vec_init(vec *v, int64_t el_size, stalloc *alloc, int32_t flags,
                 int64_t initial_size);
void  vec_free(vec *v);
void  vec_resize(vec *v, int64_t size);
void *vec_copy(vec *dest, vec *src);
void  vec_clear(vec *v);

/* Element Operations */
void   *vec_at(vec *v, int64_t pos);
void    vec_delete_at(vec *v, int64_t pos);
void    vec_put(vec *v, int64_t pos, void *el);
bool    vec_has(vec *v, void *el);
int64_t vec_find(vec *v, void *el);
void    vec_push(vec *v, void *el);
void    vec_pop(vec *v);
void   *vec_top(vec *v);
void    vec_swap(vec *v, int64_t idx1, int64_t idx2);

/* Functional Operations */
void    vec_sort(vec *v, _compare cmp, void *args);
int64_t vec_count_if(vec *v, _pred p, void *args);
vec    *vec_filter(vec *v, _pred p, void *args);
void    vec_foreach(vec *v, _each n, void *args);
vec    *vec_map(vec *v, _unary u, void *args);
void   *vec_foldl(vec *v, _binary b, void *result, void *args);

/* Vector Type Interface */
#define VEC_TYPEDEC(cn, ty)                                                    \
  typedef vec cn;                                                              \
                                                                               \
  cn *cn##_sinit(cn *v, int64_t v_size);                                       \
  cn *cn##_hinit(cn *v);                                                       \
  cn *cn##_inita(cn *v, stalloc *alloc, int8_t flags, int64_t v_size);         \
                                                                               \
  void cn##_free(cn *v);                                                       \
  void cn##_resize(cn *v, int64_t size);                                       \
  cn  *cn##_copy(cn *dest, cn *src);                                           \
                                                                               \
  void cn##_clear(cn *v);                                                      \
                                                                               \
  ty     *cn##_at(cn *v, int64_t pos);                                         \
  void    cn##_put(cn *v, int64_t pos, void *el);                              \
  void    cn##_delete_at(cn *v, int64_t pos);                                  \
  bool    cn##_has(cn *v, ty *el);                                             \
  int64_t cn##_find(cn *v, ty *el);                                            \
  void    cn##_push(cn *v, ty *el);                                            \
  void    cn##_pop(cn *v);                                                     \
  ty     *cn##_top(cn *v);                                                     \
  void    cn##_swap(cn *v, int64_t idx1, int64_t idx2);                        \
                                                                               \
  void    cn##_sort(cn *v, _compare cmp, void *args);                          \
  int64_t cn##_count_if(cn *v, _pred p, void *args);                           \
  cn     *cn##_filter(cn *v, _pred p, void *args);                             \
  void    cn##_foreach(cn *v, _each n, void *args);                            \
  cn     *cn##_map(cn *v, _unary u, void *args);                               \
  void   *cn##_foldl(cn *v, _binary b, void *result, void *args);

#define VEC_TYPE_IMPL(cn, ty)                                                    \
  typedef vec cn;                                                                \
                                                                                 \
  cn *cn##_sinit(cn *v, int64_t v_size) {                                        \
    return __vec_init((vec *)v, sizeof(ty), get_frame_ctx(), TO_STACK,           \
                      v_size);                                                   \
  }                                                                              \
  cn *cn##_hinit(cn *v) {                                                        \
    return __vec_init((vec *)v, sizeof(ty), get_frame_ctx(), TO_HEAP,            \
                      VECTOR_DEFAULT_SIZE);                                      \
  }                                                                              \
  cn *cn##_inita(cn *v, stalloc *alloc, int8_t flags, int64_t v_size) {          \
    return __vec_init((vec *)v, sizeof(ty), alloc, flags, v_size);               \
  }                                                                              \
  void cn##_free(cn *v) { vec_free((vec *)v); }                                  \
  void cn##_resize(cn *v, int64_t size) { vec_resize((vec *)v, size); }          \
  cn  *cn##_copy(cn *dest, cn *src) {                                            \
    return vec_copy((vec *)dest, (vec *)src);                                   \
  }                                                                              \
                                                                                 \
  void cn##_clear(cn *v) { vec_clear((vec *)v); }                                \
                                                                                 \
  ty     *cn##_at(cn *v, int64_t pos) { return vec_at((vec *)v, pos); }          \
  void    cn##_put(cn *v, int64_t pos, void *el) { vec_put((vec *)v, pos, el); } \
  void    cn##_delete_at(cn *v, int64_t pos) { vec_delete_at((vec *)v, pos); }   \
  bool    cn##_has(cn *v, ty *el) { return vec_has((vec *)v, el); }              \
  int64_t cn##_find(cn *v, ty *el) { return vec_find((vec *)v, el); }            \
  void    cn##_push(cn *v, ty *el) { vec_push((vec *)v, el); }                   \
  void    cn##_pop(cn *v) { vec_pop((vec *)v); }                                 \
  ty     *cn##_top(cn *v) { return vec_top((vec *)v); }                          \
  void    cn##_swap(cn *v, int64_t idx1, int64_t idx2) {                         \
    vec_swap((vec *)v, idx1, idx2);                                           \
  }                                                                              \
                                                                                 \
  void cn##_sort(cn *v, _compare cmp, void *args) {                              \
    vec_sort((vec *)v, cmp, args);                                               \
  }                                                                              \
  int64_t cn##_count_if(cn *v, _pred p, void *args) {                            \
    return vec_count_if((vec *)v, p, args);                                      \
  }                                                                              \
  cn *cn##_filter(cn *v, _pred p, void *args) {                                  \
    return vec_filter((vec *)v, p, args);                                        \
  }                                                                              \
  void cn##_foreach(cn *v, _each n, void *args) {                                \
    vec_foreach((vec *)v, n, args);                                              \
  }                                                                              \
  cn *cn##_map(cn *v, _unary u, void *args) {                                    \
    return vec_map((vec *)v, u, args);                                           \
  }                                                                              \
  void *cn##_foldl(cn *v, _binary b, void *result, void *args) {                 \
    return vec_foldl((vec *)v, b, result, args);                                 \
  }

/* =========================================================================
  Section: Map
========================================================================= */
typedef struct kvpair kvpair;
struct kvpair {
  void   *key, *value;
  int32_t state; /* 0 = free, X != 0 in use */
};

typedef struct map map;

struct map {
  int64_t __size;     /* The allocated count of elements, used internally. */
  int64_t __el_size;  /* The size of a element */
  int64_t __key_size; /* The size of a key */

  int64_t  slots_in_use;  /* The count of elements in the map. */
  int32_t  flags;         /* Contains allocation context information. */
  int32_t  in_use_id;     /* Each clear increments this. */
  int32_t  cache_counter; /* Increments each resize of invalidation checks. */
  stalloc *allocator;
  vec      elements;
};

/* Container Operations */
map *__map_init(map *m, int64_t el_size, int64_t key_size, stalloc *alloc,
                int32_t flags, int64_t intial_size);

void    map_free(map *m);
void    map_clear(map *m);
vec    *map_to_vec(map *m, vec *out);
map    *map_copy(map *dest, map *src);
int64_t map_load(map *m);

/* Element Operations */
kvpair map_get(map *m, void *key);
void   map_put(map *m, void *key, void *value);
bool   map_has(map *m, void *key);
void   map_del(map *m, void *key);
kvpair read_kvpair(map *m, void *el);

/* Functional Operations */
int64_t map_count_if(map *m, _pred p, void *args);
void    map_foreach(map *m, _each n, void *args);
map    *map_filter(map *m, _pred p, void *args);
kvpair  map_find_one(map *m, _pred p, void *args);

/* Map Type Interface */
#define MAP_TYPEDEC(cn, keyty, ty)                                             \
  typedef map cn;                                                              \
                                                                               \
  cn *cn##_sinit(cn *m, int64_t initial_size);                                 \
  cn *cn##_hinit(cn *m);                                                       \
  cn *cn##_inita(cn *m, stalloc *alloc, int8_t flags, int64_t initial_size);   \
                                                                               \
  /* Container Operations */                                                   \
  void  cn##_free(cn *m);                                                      \
  void  cn##_clear(cn *m);                                                     \
  vec  *cn##_to_vec(cn *m, vec *out);                                          \
  void *cn##_copy(cn *dest, cn *src);                                          \
                                                                               \
  int64_t cn##_load(cn *m);                                                    \
                                                                               \
  /* Element Operations */                                                     \
  kvpair cn##_get(cn *m, void *key);                                           \
  void   cn##_put(cn *m, void *key, void *value);                              \
  bool   cn##_has(cn *m, void *key);                                           \
  void   cn##_del(cn *m, void *key);                                           \
                                                                               \
  /* Functional Operations */                                                  \
  int64_t cn##_count_if(cn *m, _pred p, void *args);                           \
  void    cn##_foreach(cn *m, _each n, void *args);                            \
  cn     *cn##_filter(cn *m, _pred p, void *args);                             \
  kvpair  cn##_find_one(cn *m, _pred p, void *args);

#define MAP_TYPE_IMPL(cn, keyty, ty)                                           \
  typedef map cn;                                                              \
                                                                               \
  cn *cn##_sinit(cn *m, int64_t initial_size) {                                \
    return __map_init((map *)m, sizeof(ty), sizeof(keyty), get_frame_ctx(),    \
                      TO_STACK, initial_size);                                 \
  }                                                                            \
                                                                               \
  cn *cn##_hinit(cn *m) {                                                      \
    return __map_init((map *)m, sizeof(ty), sizeof(keyty), get_frame_ctx(),    \
                      TO_HEAP, MAP_DEFAULT_SIZE);                              \
  }                                                                            \
                                                                               \
  cn *cn##_inita(cn *m, stalloc *alloc, int8_t flags, int64_t initial_size) {  \
    return __map_init((map *)m, sizeof(ty), sizeof(keyty), alloc, flags,       \
                      initial_size);                                           \
  }                                                                            \
                                                                               \
  /* Container Operations */                                                   \
  void  cn##_free(cn *m) { map_free((map *)m); }                               \
  void  cn##_clear(cn *m) { map_clear((map *)m); }                             \
  vec  *cn##_to_vec(cn *m, vec *out) { return map_to_vec((map *)m, out); }     \
  void *cn##_copy(cn *dest, cn *src) {                                         \
    return map_copy((map *)dest, (map *)src);                                  \
  }                                                                            \
                                                                               \
  int64_t cn##_load(cn *m) { return map_load((map *)m); }                      \
                                                                               \
  /* Element Operations */                                                     \
  kvpair cn##_get(cn *m, void *key) { return map_get((map *)m, key); }         \
  void   cn##_put(cn *m, void *key, void *value) {                             \
    map_put((map *)m, key, value);                                           \
  }                                                                            \
  bool cn##_has(cn *m, void *key) { return map_has((map *)m, key); }           \
  void cn##_del(cn *m, void *key) { map_del((map *)m, key); }                  \
                                                                               \
  /* Functional Operations */                                                  \
  int64_t cn##_count_if(cn *m, _pred p, void *args) {                          \
    return map_count_if((map *)m, p, args);                                    \
  }                                                                            \
  void cn##_foreach(cn *m, _each n, void *args) {                              \
    map_foreach((map *)m, n, args);                                            \
  }                                                                            \
  cn *cn##_filter(cn *m, _pred p, void *args) {                                \
    return map_filter((map *)m, p, args);                                      \
  }                                                                            \
  kvpair cn##_find_one(cn *m, _pred p, void *args) {                           \
    return map_find_one((map *)m, p, args);                                    \
  }

/* =========================================================================
  Section: Set
========================================================================= */
typedef struct set set;
struct set {
  map internals; /* A set is just an open address map. */
};

/* Container Operations */
set    *__set_init(set *s, int64_t el_size, stalloc *alloc, int32_t flags,
                   int64_t initial_size);
void    set_free(set *s);
void    set_clear(set *s);
set    *set_copy(set *dest, set *src);
int64_t set_length(set *s);

set *set_intersect(set *a, set *b, set *out);
set *set_union(set *a, set *b, set *out);

/* Element Operations */
void set_put(set *s, void *key);
bool set_has(set *s, void *key);
void set_del(set *s, void *key);

#define SET_TYPEDEC(cn, ty)                                                    \
  typedef set cn;                                                              \
                                                                               \
  cn *cn##_sinit(cn *s, int64_t initial_size);                                 \
  cn *cn##_hinit(cn *s);                                                       \
  cn *cn##_inita(cn *s, stalloc *alloc, int8_t flags, int64_t initial_size);   \
                                                                               \
  /* Container Operations */                                                   \
  void    cn##_free(cn *s);                                                    \
  void    cn##_clear(cn *s);                                                   \
  void    cn##_copy(cn *dest, cn *src);                                        \
  int64_t cn##_length(cn *s);                                                  \
                                                                               \
  cn *cn##_intersect(cn *a, cn *b, cn *out);                                   \
  cn *cn##_union(cn *a, cn *b, cn *out);                                       \
                                                                               \
  void cn##_put(cn *s, void *key);                                             \
  bool cn##_has(cn *s, void *key);                                             \
  void cn##_del(cn *s, void *key);

#define SET_TYPE_IMPL(cn, ty)                                                  \
  typedef set cn;                                                              \
                                                                               \
  cn *cn##_sinit(cn *s, int64_t initial_size) {                                \
    return __set_init((set *)s, sizeof(ty), get_frame_ctx(), TO_STACK,         \
                      initial_size);                                           \
  }                                                                            \
                                                                               \
  cn *cn##_hinit(cn *s) {                                                      \
    return __set_init((set *)s, sizeof(ty), get_frame_ctx(), TO_HEAP,          \
                      MAP_DEFAULT_SIZE);                                       \
  }                                                                            \
                                                                               \
  cn *cn##_inita(cn *s, stalloc *alloc, int8_t flags, int64_t initial_size) {  \
    return __set_init((set *)s, sizeof(ty), alloc, flags, initial_size);       \
  }                                                                            \
                                                                               \
  /* Container Operations */                                                   \
  void    cn##_free(cn *s) { set_free((set *)s); }                             \
  void    cn##_clear(cn *s) { set_clear((set *)s); }                           \
  void    cn##_copy(cn *dest, cn *src) { set_copy((set *)dest, (set *)src); }  \
  int64_t cn##_length(cn *s) { return set_length((set *)s); }                  \
                                                                               \
  cn *cn##_intersect(cn *a, cn *b, cn *out) {                                  \
    return (cn *)set_intersect((set *)a, (set *)b, (set *)out);                \
  }                                                                            \
  cn *cn##_union(cn *a, cn *b, cn *out) {                                      \
    return (cn *)set_union((set *)a, (set *)b, (set *)out);                    \
  }                                                                            \
                                                                               \
  void cn##_put(cn *s, void *key) { set_put((set *)s, key); }                  \
  bool cn##_has(cn *s, void *key) { return set_has((set *)s, key); };          \
  void cn##_del(cn *s, void *key) { set_del((set *)s, key); }

/* =========================================================================
  Section: Cbuff
========================================================================= */
struct cbuff {
  void   *region; /* Pointer to base memory. */
  void   *top;    /* Pointer to the end of base memory operations. */
  int64_t len;    /* Length of objects pushed/pop'd. */
};

/* Container Operations */
cbuff *buff_init(cbuff *b, void *region);

/* Element Operations */
void  buff_push(cbuff *b, void *item, size_t item_size);
void *buff_pop(cbuff *b, size_t item_size);
void *buff_at(cbuff *b);
void *buff_skip(cbuff *b, size_t to_skip);

/* =========================================================================
  Section: Utilities
========================================================================= */
void     memswap(void *a, void *b, size_t size);
void    *recalloc(void *a, size_t size);
uint64_t hash_bytes(void *ptr, size_t size);

#endif