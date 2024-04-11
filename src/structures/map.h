#ifndef __HEADER_MAP_H__
#define __HEADER_MAP_H__

#include <stdbool.h>
#include <stdlib.h>

#define MAP_FAIL -1
#define MAP_OKAY 0
#define MAP_SLOTS_START 67

typedef struct map_t     map_t;
typedef struct kv_pair_t kv_pair_t;
struct kv_pair_t {
  void *key;
  void *value;

  size_t key_size;
};

map_t *map_make(size_t el_size);
int    map_add(map_t *map, kv_pair_t *pair);
void  *map_find(map_t *map, void *key, size_t key_size);
bool   map_has(map_t *map, void *key, size_t key_size);

#endif