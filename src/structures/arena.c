/*==============================================================================
Associated Files:
    arena.c arena.h
Purpose:
    Arena alloc structure implementation. Uses a region-based linked-list to
    implement a lifetime collection of heap objects.
==============================================================================*/
#include "arena.h"

#ifndef DISABLE_ARENA

/*---- DECLARATIONS ----------------------------------------------------------*/
typedef struct region region;

struct arena {
  /* Pointers to the beginning and ending of the linked-list of regions. */
  region *begin, *end;
  /* The count of regions in this arena. */
  size_t region_cnt;
};

/**
 * @brief The definition of a contiguous region in memory
 */
struct region {
  /* Last item that was allocated. */
  size_t alloc_pos;
  /* Base pointer to memory in heap. */
  void *base_memory;
  /* Next region. */
  region *next;
};

/**
 * @brief Create a new region in the arena.
 * @return region* to new region.
 */
static region *new_region(void);

/**
 * @brief Ensure that there is sufficient space reserved in the arena. Does
 * nothing if `size` fits in existing region.
 * @param a    the arena.
 * @param size the size.
 */
static void reserve(arena *a, size_t size);

/**
 * @brief Destroy a region and its subsequent regions. Use mainly to free.
 * @param r The region to free.
 */
static void region_destroy(region *r);

/*---- IMPLEMENTATION ----------------------------------------------*/
arena *arena_make(void) {
  arena  *a;
  region *r;

  a = malloc(sizeof(arena));
  assert(a);
  r = new_region();

  /* Initialize arena fields. */
  a->begin = r;
  a->end = r;
  a->region_cnt = 1;

  return a;
}

void arena_destroy(arena *a) {
  assert(a);
  region_destroy(a->begin);
  free(a);
}

void *arena_alloc(arena *a, size_t size) {
  assert(a);
  assert(a->begin);
  assert(a->end);
  assert(size <= REGION_SIZE);

  /* Memory pointer to the start of the allocation in the arena. */
  void *ret;

  /* Since we assert size <= REGION_STATE. reserve guarentees we will have
  space for the proper location. */
  reserve(a, size);
  ret = a->end->base_memory + a->end->alloc_pos;
  a->end->alloc_pos += size;
  return ret;
}

int arena_poll(arena *a) {
  assert(a);
  return REGION_SIZE - a->end->alloc_pos;
}

void arena_refresh(arena *a) {
  assert(a);
  assert(a->begin);
  assert(a->end);

  region *r = new_region();
  a->end->next = r;
  a->end = r;
}

/*---- STATIC FUNCTIONS ------------------------------------------------------*/
static region *new_region(void) {
  region *r = malloc(sizeof(region));
  void   *r_mem = malloc(REGION_SIZE);
  assert(r);
  assert(r_mem);

  /* Initialize new region fields. */
  r->alloc_pos = 0;
  r->base_memory = r_mem;
  r->next = NULL;

  return r;
}

static void region_destroy(region *r) {
  if (r == NULL) return;
  region_destroy(r->next);
  free(r->base_memory);
  free(r);
}

static void reserve(arena *a, size_t size) {
  /* To preserve speed, we alloc a new region without checking previous
     regions if they have the required space for the incoming object. We do
     this instead of linear checking or using a table for the following
     reasons:
          - We assume that when a user of the library calls `alloc`, that
            these objects will be accessed close to each other in the code. If
            we assume this, it would make no sense to fragment the access to
            preserve space.
          - Even though this approach leads to fragmentation, we are forbidden
            using the realloc call because all previously given pointers to
            the user will be invalid. */
  size_t next_alloc_pos = a->end->alloc_pos + size;
  if (next_alloc_pos > REGION_SIZE) {
    region *r = new_region();
    a->end->next = r;
    a->end = r;
  }
}
#else
/* The following removes the arena to allow for easy debugging with external
tooling. */
struct arena {
  /* We preserve the arena's API by keeping track of the mallocs and then
     freeing them all at once. */
  vector *mallocs;
};
arena *arena_make(void) {
  arena *a;
  a = malloc(sizeof(a));
  a->mallocs = vector_make(sizeof(size_t));
  assert(a);
  return a;
}
void arena_destroy(arena *a) {
  for (int i = 0; i < vector_len(a->mallocs); i++) {
    free(*(void **)vector_at(a->mallocs, i));
  }
  vector_free(a->mallocs);
  free(a);
  return;
}
void *arena_alloc(arena *a, size_t bytes) {
  void *p = malloc(bytes);
  vector_push(a->mallocs, &p);
  return p;
}
int  arena_poll(arena *a) { return 0; }
void arena_refresh(arena *a) { return; }
#endif
