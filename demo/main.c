#include <stdio.h>

#include "map.h"

typedef struct vec2d vec2d;
struct vec2d {
  int x, y;
};

MAP_GEN_H(int, vec2d);
MAP_GEN_C(int, vec2d);

int main(void) {
  int_vec2d_map_t *map = int_vec2d_map_heap_init();

  // for (int i = 0; i < 500; i++) {
  //   int_vec2d_map_put(map, &i, &(vec2d){.x = i, .y = i});
  // }

  int_vec2d_map_heap_free(map);

  return 0;
}