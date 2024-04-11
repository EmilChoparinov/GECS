#include <stdio.h>

#include "arena.h"
#include "gecs.h"
#include "map.h"
#include "vector.h"

void test_vector(void) {
  vector_t *v = vector_make(sizeof(int));

  for (int i = 0; i < 32 * 10; i++) {
    vector_push(v, &i);
  }

  printf("first el: %d\n", *(int *)vector_first(v));
  printf("last el: %d\n", *(int *)vector_last(v));

  for (int i = 0; i < vector_len(v); i++) {
    printf("%d\n", *(int *)vector_at(v, i));
  }

  vector_free(v);
}

void test_arena(void) {
  arena *alloc = arena_make();
  printf("%ld\n", sizeof(void *));

  int *p1 = arena_alloc(alloc, sizeof(int));
  *p1 = 0x5;
  int *p2 = arena_alloc(alloc, sizeof(int));
  *p2 = 0x6;
  int *p3 = arena_alloc(alloc, sizeof(int));
  *p3 = 0x7;

  printf("%d\n%d\n%d\n", *p1, *p2, *p3);
  arena_destroy(alloc);
}

typedef struct {
  int x;
  int y;
} mat_2d_t;

void test_world(void) {
  gecs_core_t *world = gecs_make_world();

  GECS_REG_COMPONENT(world, mat_2d_t);

  gecs_component_info_t *x =
      (gecs_component_info_t *)vector_at(world->components_registry, 0);

  printf("%lu\n", x->id);
  printf("sizeof: %zu\n", sizeof(mat_2d_t));

  for (int i = 0; i < 2; i++) gecs_progress(world);

  gecs_complete(world);
}

void test_map(void) {
  map_t *hmap = map_make(sizeof(mat_2d_t));

  kv_pair_t test = {
    .key = "testing",
    .key_size = sizeof("testing"),
    .value = NULL
  };

  map_add(hmap, &(kv_pair_t){.key = "hello",
                             .key_size = sizeof("hello"),
                             .value = &(mat_2d_t){.x = 5, .y = 5}});
  map_add(hmap, &(kv_pair_t){.key = "world",
                             .key_size = sizeof("world"),
                             .value = &(mat_2d_t){.x = 420, .y = 69}});
  map_add(hmap, &test);

  mat_2d_t *first = map_find(hmap, "hello", sizeof("hello"));
  mat_2d_t *second = map_find(hmap, "world", sizeof("world"));

  if(first == NULL) printf("first is null\n");
  if(second == NULL) printf("second is null\n");

  printf("%d,%d\n%d,%d\n", first->x, first->y, second->x, second->y);
}

int main() {
  // test_vector();
  // test_arena();
  test_map();

  // test_world();
  return 0;
}