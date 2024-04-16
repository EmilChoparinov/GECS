#include <string.h>

#include "map.h"
#include "vector.h"

static const unsigned long hash(const char *str);

struct map_t {
  void **slots;

  size_t value_size;
  size_t len;
};

static char *key_to_hash(map_t *map, void *key, size_t key_size);

map_t *map_make(size_t value_size) {
  map_t *map;

  if ((map = malloc(sizeof(*map))) == NULL) return NULL;

  if ((map->slots = (void **)calloc(MAP_SLOTS_START, sizeof(void *))) == NULL)
    return NULL;

  map->value_size = value_size;
  map->len = MAP_SLOTS_START;

  return map;
}

int map_add(map_t *map, kv_pair_t *pair) {
  unsigned long loc;
  vector_t     *slot;
  char         *to_hash;

  if (pair->key == NULL || pair->value == NULL) return MAP_FAIL;

  if ((to_hash = key_to_hash(map, pair->key, pair->key_size)) == NULL)
    return MAP_FAIL;

  // Create hash, ensure slot exists and vector_make succeeded
  loc = hash(to_hash) % map->len;
  if (map->slots[loc] == NULL) map->slots[loc] = vector_make(sizeof(kv_pair_t));
  if (map->slots[loc] == NULL) return MAP_FAIL;

  slot = (vector_t *)map->slots[loc];

  vector_push(slot, pair);

  free(to_hash);

  return MAP_OKAY;
}

void *map_find(map_t *map, void *key, size_t key_size) {
  char         *to_hash;
  unsigned long loc;
  vector_t     *slot;
  size_t        i;
  kv_pair_t    *kv;

  if ((to_hash = key_to_hash(map, key, key_size)) == NULL) return NULL;

  loc = hash(to_hash) % map->len;
  if (map->slots[loc] == NULL) return NULL;
  slot = map->slots[loc];

  for (i = 0; i < vector_len(slot); i++) {
    kv = (kv_pair_t *)vector_at(slot, i);
    // TODO: read vuln, make it so that it takes the min keysize, either in kv
    // TODO: or key_size
    if (memcmp(kv->key, key, key_size) == 0) break;
  }

  free(to_hash);

  if (i == vector_len(slot)) return NULL;
  return kv->value;
}

bool map_has(map_t *map, void *key, size_t key_size) {
  void *result;

  result = map_find(map, key, key_size);
  return result != NULL;
}

static const unsigned long hash(const char *str) {
  /* djb2 by Dan Bernstein
   * http://www.cse.yorku.ca/~oz/hash.html */
  unsigned long hash = 5381;
  int           c;

  while ((c = *str++)) hash = ((hash << 5) + hash) + c;
  return hash;
}

static char *key_to_hash(map_t *map, void *key, size_t key_size) {
  char *to_hash;
  if ((to_hash = malloc(key_size + 1)) == NULL) return NULL;
  memcpy(to_hash, key, key_size);
  to_hash[key_size] = 0;
  return to_hash;
}