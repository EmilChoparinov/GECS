#include <stdio.h>

#include "arena.h"
#include "gecs.h"
#include "nmap.h"
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

// void test_map(void) {
//   map_t *hmap = map_make(sizeof(mat_2d_t));

//   kv_pair_t test = {
//       .key = "testing", .key_size = sizeof("testing"), .value = NULL};

//   map_add(hmap, &(kv_pair_t){.key = "hello",
//                              .key_size = sizeof("hello"),
//                              .value = &(mat_2d_t){.x = 5, .y = 5}});
//   map_add(hmap, &(kv_pair_t){.key = "world",
//                              .key_size = sizeof("world"),
//                              .value = &(mat_2d_t){.x = 420, .y = 69}});
//   map_add(hmap, &test);

//   mat_2d_t *first = map_find(hmap, "hello", sizeof("hello"));
//   mat_2d_t *second = map_find(hmap, "world", sizeof("world"));

//   if (first == NULL) printf("first is null\n");
//   if (second == NULL) printf("second is null\n");

//   printf("%d,%d\n%d,%d\n", first->x, first->y, second->x, second->y);
// }

void test_nmap(void) {
  int     zero = 0;
  int     one = 1;
  int     two = 2;
  int     three = 3;
  nmap_t *map = nmap_make(sizeof(int), sizeof(mat_2d_t), 64);
  nmap_add(map, &(nmap_keypair_t){.key = (void *)&zero,
                                  .value = &(mat_2d_t){.x = 6, .y = 9}});
  nmap_add(map, &(nmap_keypair_t){.key = (void *)&one,
                                  .value = &(mat_2d_t){.x = 7, .y = 8}});
  nmap_add(map, &(nmap_keypair_t){.key = (void *)&two,
                                  .value = &(mat_2d_t){.x = 8, .y = 7}});
  nmap_add(map, &(nmap_keypair_t){.key = (void *)&three,
                                  .value = &(mat_2d_t){.x = 9, .y = 6}});

  mat_2d_t *first = nmap_find(map, &zero);
  mat_2d_t *third = nmap_find(map, &two);
  mat_2d_t *last = nmap_find(map, &three);

  if (first == NULL) printf("first is null\n");
  if (third == NULL) printf("third is null\n");
  if (last == NULL) printf("last is null\n");

  printf("%d,%d\n%d,%d\n%d,%d", first->x, first->y, third->x, third->y, last->x,
         last->y);

  nmap_remove(map, &two);
  nmap_remove(map, &one);
  nmap_remove(map, &three);
  nmap_remove(map, &zero);
  nmap_free(map);
}