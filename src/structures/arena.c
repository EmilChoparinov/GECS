#include "arena.h"

#define ON_MALLOC_FAIL                                                         \
  log_error("Malloc failed, exiting.");                                        \
  exit(EXIT_FAILURE);

/*-------------------------------------------------------
 * DATA STRUCTURES
 *-------------------------------------------------------
 * The arena datastructure is essentially a linked list
 * that starts at *first and ends at *last.
 */

typedef struct region_t region_t;

struct region_t {
  size_t    alloc_curs;  /* Relative curs pointing to next available address. */
  size_t    region_size; /* How much memory is allocated in this region. */
  void     *memory;
  region_t *next;
};

struct arena_t {
  region_t *last; /* Points to the last chunk allocated for O(1) access */
  region_t  first;
};

/*-------------------------------------------------------
 * REGION DECLARATIONS
 *-------------------------------------------------------*/
static void region_init(region_t *reg, size_t bytes);
static void region_free(region_t *reg);

/*-------------------------------------------------------
 * CONTAINER OPERATIONS
 *-------------------------------------------------------*/
arena_t *arena_init(void) {
  arena_t *ar;

  /* Since we allocate with arena_t the first region, we point to the internal
     address in the struct to initialize last. */
  ar = malloc(sizeof(*ar));
  if (!ar) ON_MALLOC_FAIL;
  ar->last = &ar->first;

  region_init(ar->last, 0);

  return ar;
}

void arena_free(arena_t *ar) {
  assert(ar);
  region_t *current = ar->first.next;

  region_t *swap;
  while (current) {
    swap = current;
    current = current->next;

    region_free(swap);
    free(swap);
  }

  free(ar->first.memory);
  free(ar);
}

/*-------------------------------------------------------
 * ELEMENT OPERATIONS
 *-------------------------------------------------------*/
void *arena_alloc(arena_t *ar, size_t bytes) {
  assert(ar);
  assert(ar->last->next == NULL && "Structure corrupted");

  /* If we have no more space, we push a new region. */
  if (bytes > arena_poll(ar)) arena_push_region(ar, bytes);

  void *loc; /* The start address of available memory in this chunk */

  loc = ar->last->memory + ar->last->alloc_curs;
  ar->last->alloc_curs += bytes;
  return loc;
}

void *arena_realloc(arena_t *ar, void *ptr, size_t bytes) {
  assert(ar);
  assert(ar->last->next == NULL && "Structure corrupted");
  return NULL;
}

void arena_push_region(arena_t *ar, size_t chunk_size) {
  assert(ar);
  assert(ar->last->next == NULL && "Structure corrupted");

  ar->last->next = malloc(sizeof(*ar->last->next));
  if (!ar->last->next) ON_MALLOC_FAIL;
  region_init(ar->last->next, chunk_size);
  ar->last = ar->last->next;
}

size_t arena_poll(arena_t *ar) {
  return ar->last->region_size - ar->last->alloc_curs;
}

/*-------------------------------------------------------
 * REGION OPERATIONS
 *-------------------------------------------------------*/
static void region_init(region_t *reg, size_t bytes) {
  assert(reg);
  size_t to_reserve = bytes;
  if (to_reserve < REGION_SIZE) to_reserve = REGION_SIZE;

  reg->memory = malloc(to_reserve);
  if (!reg->memory) ON_MALLOC_FAIL;
  reg->alloc_curs = 0;
  reg->region_size = to_reserve;
  reg->next = NULL;
}

static void region_free(region_t *reg) { free(reg->memory); }