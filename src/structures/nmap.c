#include "nmap.h"

struct nmap_t {
  char           *in_use;
  nmap_keypair_t *slots;

  size_t slots_used;
  size_t capacity;

  size_t key_size;
  size_t value_size;
};

static int nmap_find_index(nmap_t *map, void *key);
static int nmap_hash_index(nmap_t *map, void *key);
static int ensure_lf_constraint(nmap_t *map);

nmap_t *nmap_make(size_t key_size, size_t value_size, size_t initial_map_size) {
  nmap_t *map;
  size_t  itr;

  if ((map = malloc(sizeof(*map))) == NULL) return NULL;
  map->value_size = value_size;
  map->key_size = key_size;
  map->slots_used = 0;
  map->capacity = initial_map_size;

  // Set all slots to unused
  if ((map->in_use = calloc(map->capacity, sizeof(char))) == NULL) return NULL;
  if ((map->slots = malloc(map->capacity * (sizeof(nmap_keypair_t) +
                                            value_size + key_size))) == NULL)
    return NULL;

  // Link pointers into malloc'd mem
  char *curs = (char *)(map->slots + map->capacity);
  for (itr = 0; itr < map->capacity; itr++) {
    map->slots[itr].key = curs;
    curs += key_size;
    map->slots[itr].value = curs;
    curs += map->value_size;
  }

  return map;
}

int nmap_free(nmap_t *map) {
  free(map->in_use);
  free(map->slots);
  free(map);
  return NMAP_OK;
}

void *nmap_find(nmap_t *map, void *key) {
  int find_index;

  find_index = nmap_find_index(map, key);
  if (!map->in_use[find_index]) return NULL;

  return map->slots[find_index].value;
}

int nmap_add(nmap_t *map, nmap_keypair_t *pair) {
  int hash_index, start_at;
  if (ensure_lf_constraint(map) == NMAP_FAIL) return NMAP_FAIL;

  if ((hash_index = nmap_hash_index(map, pair->key)) == NMAP_FAIL)
    return NMAP_FAIL;

  start_at = hash_index;

  // Linear probe to end until we find an empty slot
  while (start_at < map->capacity && map->in_use[start_at]) ++start_at;

  // If we didnt expend the list, we found the location. Else, start from
  // beginning of list and loop back to the hash index.
  if (start_at == map->capacity) start_at = 1;

  // Linear probe to hash_index until we find an empty slot
  while (start_at < hash_index && map->in_use[start_at]) ++start_at;

  // We are guarenteed to have space because of the load factor, so we have
  // no issue just doing this here like so:
  memcpy(map->slots[start_at].key, pair->key, map->key_size);
  memcpy(map->slots[start_at].value, pair->value, map->value_size);
  map->in_use[start_at] = true;
  map->slots_used++;

  return NMAP_OK;
}

int nmap_remove(nmap_t *map, void *key) {
  int find_index;

  find_index = nmap_find_index(map, key);
  if (find_index == NMAP_FAIL) return NMAP_FAIL;
  map->in_use[find_index] = false;
  return NMAP_OK;
}

static const unsigned long hash(const char *str) {
  /* djb2 by Dan Bernstein
   * http://www.cse.yorku.ca/~oz/hash.html */
  unsigned long hash = 5381;
  int           c;

  while ((c = *str++)) hash = ((hash << 5) + hash) + c;
  return hash;
}

static int ensure_lf_constraint(nmap_t *map) {
  if ((float)map->slots_used / map->capacity < NMAP_LOAD_FACTOR) return NMAP_OK;

  nmap_t *temp_map;
  int     old_index;

  temp_map = nmap_make(map->key_size, map->value_size, map->capacity * 2);

  for (old_index = 0; old_index < map->capacity; old_index++) {
    if (!map->in_use[old_index]) continue;
    if (nmap_add(temp_map, &(nmap_keypair_t){
                               .key = map->slots[old_index].key,
                               .value = map->slots[old_index].value,
                           }) == NMAP_FAIL)
      return NMAP_FAIL;
  }

  free(map->in_use);
  free(map->slots);

  map->capacity *= 2;
  map->in_use = temp_map->in_use;
  map->slots = temp_map->slots;

  free(temp_map);

  return NMAP_OK;
}

static int nmap_hash_index(nmap_t *map, void *key) {
  char *byte_key;
  int   hash_key;

  // Process key into raw bytes
  if ((byte_key = malloc(map->key_size + 1)) == NULL) return NMAP_FAIL;
  memcpy(byte_key, key, map->key_size);
  byte_key[map->key_size] = 0;

  hash_key = hash(byte_key) % map->capacity;

  free(byte_key);
  return hash_key;
}

static int nmap_find_index(nmap_t *map, void *key) {
  int hash_index, start_at;

  if ((hash_index = nmap_hash_index(map, key)) == NMAP_FAIL) return NMAP_FAIL;

  // Linear probe from the hash index to end
  start_at = hash_index;
  while (start_at < map->capacity &&
         memcmp(map->slots[start_at].key, key, map->key_size) != 0)
    ++start_at;

  // If start at is not at the end of the list, we found it and can return the
  // index, else, we search from the beginning to the hash index
  if (start_at != map->capacity) return start_at;

  start_at = 1;
  while (start_at < hash_index &&
         memcmp(map->slots[start_at].key, key, map->key_size) != 0)
    ++start_at;

  // If start at is not at the hash index, we found it and can return the
  // index. Else, we failed to find the key in the list through a full scan
  if (start_at != hash_index) return start_at;

  return NMAP_FAIL;
}