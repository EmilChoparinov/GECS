#include "nmap.h"
#include "shortcuts.h"
#include "unity.h"

//== REGISTER ==================================================================
BOILER();

/* Test creation and deletion logic */
REGISTER(create_and_delete);
/* Test adding to a static map */
REGISTER(add_no_resize);
/* Test removing on a static map */
REGISTER(remove_no_resize);
/* Test adding on a dynamic map */
REGISTER(add_resize);
/* Test removing on a dynamic map */
REGISTER(remove_resize);
/* Test on returning undefined and defined keys */
REGISTER(key_querying);
/* Testing to make sure variables are copied to the structures memory */
REGISTER(variables_copied_correctly);

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(create_and_delete);
  RUN_TEST(add_no_resize);
  RUN_TEST(remove_no_resize);
  RUN_TEST(add_resize);
  RUN_TEST(remove_resize);
  RUN_TEST(key_querying);
  RUN_TEST(variables_copied_correctly);

  return UNITY_END();
}

//== TESTS =====================================================================
void create_and_delete(void) {
  nmap_t *map = nmap_make(sizeof(int), sizeof(int), 1);

  TEST_ASSERT(map != NULL);

  TEST_ASSERT(nmap_free(map) == NMAP_OK);
}

void add_no_resize(void) {
  nmap_t *map = nmap_make(sizeof(int), sizeof(int), 64);

  for (int i = 0; i < 64 * NMAP_LOAD_FACTOR; i++) {
    int stored_value = i + 1;
    nmap_add(map, &(nmap_keypair_t){.key = &i, .value = &stored_value});
  }

  for (int i = 0; i < 64 * NMAP_LOAD_FACTOR; i++) {
    int *value = nmap_find(map, &i);
    TEST_ASSERT(value != NULL);
    TEST_ASSERT(*value == i + 1);
  }

  nmap_free(map);
}

void remove_no_resize(void) {
  nmap_t *map = nmap_make(sizeof(int), sizeof(int), 64);

  for (int i = 0; i < 64 * NMAP_LOAD_FACTOR; i++) {
    int stored_value = i + 1;
    nmap_add(map, &(nmap_keypair_t){.key = &i, .value = &stored_value});
  }

  for (int i = 0; i < 64 * NMAP_LOAD_FACTOR; i++) {
    int success = nmap_remove(map, &i);
    TEST_ASSERT(success == NMAP_OK);
    int *value = nmap_find(map, &i);
    TEST_ASSERT(value == NULL);
  }

  nmap_free(map);
}

void add_resize(void) {
  nmap_t *map = nmap_make(sizeof(int), sizeof(int), 2);

  for (int i = 0; i < 64; i++) {
    int stored_value = i + 1;
    nmap_add(map, &(nmap_keypair_t){.key = &i, .value = &stored_value});
  }

  for (int i = 0; i < 64; i++) {
    int *value = nmap_find(map, &i);
    TEST_ASSERT(value != NULL);
    TEST_ASSERT(*value == i + 1);
  }

  nmap_free(map);
}

void remove_resize(void) {
  nmap_t *map = nmap_make(sizeof(int), sizeof(int), 1);

  for (int i = 0; i < 64; i++) {
    int stored_value = i + 1;
    nmap_add(map, &(nmap_keypair_t){.key = &i, .value = &stored_value});
  }

  for (int i = 0; i < 64; i++) {
    int success = nmap_remove(map, &i);
    TEST_ASSERT(success == NMAP_OK);
    int *value = nmap_find(map, &i);
    TEST_ASSERT(value == NULL);
  }

  nmap_free(map);
}

void key_querying(void) {
  nmap_t *map = nmap_make(sizeof(int), sizeof(int), 1);

  int key = 5;
  int value = 6;

  int *ret = nmap_find(map, &key);
  TEST_ASSERT(ret == NULL);
  nmap_add(map, &(nmap_keypair_t){.key = &key, .value = &value});
  ret = nmap_find(map, &key);
  TEST_ASSERT(ret != NULL);
  TEST_ASSERT(*ret == value);

  nmap_free(map);
}

void variables_copied_correctly(void) {
  nmap_t *map = nmap_make(sizeof(int), sizeof(int), 1);

  int key = 5;
  int value = 6;

  int *ret = nmap_find(map, &key);

  TEST_ASSERT(ret == NULL);

  nmap_add(map, &(nmap_keypair_t){.key = &key, .value = &value});
  value++;
  ret = nmap_find(map, &key);

  TEST_ASSERT(ret != NULL);
  TEST_ASSERT(*ret != value);
  nmap_free(map);
}