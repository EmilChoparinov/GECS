// #include "map.h"
#include "logger.h"
#include "map.h"
#include "unity.h"
#include <stdbool.h>

typedef struct complex_struct cs;
struct complex_struct {
  int  x, y, z;
  bool active;
};

typedef struct id id;
struct id {
  int  uid;
  bool is_active;
} __attribute__((packed));

MAP_GEN_H(id, cs);
MAP_GEN_C(id, cs);

/*-------------------------------------------------------
 * REGISTER
 *-------------------------------------------------------*/

id_cs_map_t tmap;

void setUp(void) { id_cs_map_init(&tmap); }
void tearDown(void) { id_cs_map_free(&tmap); }

void test_put_and_find(void);
void test_has(void);
void test_remove(void);
void test_push_loadfactor(void);

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_put_and_find);
  RUN_TEST(test_has);
  RUN_TEST(test_remove);
  RUN_TEST(test_push_loadfactor);

  UNITY_END();
}

/*-------------------------------------------------------
 * TESTS
 *-------------------------------------------------------*/
void test_put_and_find(void) {
  id key = {.is_active = true, .uid = 69};
  id fake_key = {.is_active = true, .uid = 99};
  cs value = {.x = 99};
  id_cs_map_put(&tmap, &key, &value);

  TEST_ASSERT_MESSAGE(
      memcmp(&id_cs_map_find(&tmap, &key)->value, &value, sizeof(value)) == 0,
      "Did not retrieve from map value currently put in.");

  TEST_ASSERT_MESSAGE(id_cs_map_find(&tmap, &fake_key) == NULL,
                      "Key should not have returned");
}
void test_has(void) {
  id key = {.is_active = true, .uid = 69};
  id fake_key = {.is_active = true, .uid = 99};
  cs value = {.x = 99};
  id_cs_map_put(&tmap, &key, &value);

  TEST_ASSERT_MESSAGE(id_cs_map_has(&tmap, &key),
                      "Map should contain this keypair.");

  TEST_ASSERT_MESSAGE(!id_cs_map_has(&tmap, &fake_key),
                      "Map should NOT contain this keypair.");
}
void test_remove(void) {
  id key = {.is_active = true, .uid = 69};
  cs value = {.x = 99};
  id_cs_map_put(&tmap, &key, &value);

  TEST_ASSERT_MESSAGE(id_cs_map_has(&tmap, &key),
                      "Map should contain this keypair.");

  id_cs_map_remove(&tmap, &key);
  TEST_ASSERT_MESSAGE(!id_cs_map_has(&tmap, &key),
                      "Map should NOT contain this keypair.");
}

void test_push_loadfactor(void) {
  for (int i = 0; i < 500; i++) {
    id_cs_map_put(&tmap, &(id){.is_active = true, .uid = i},
                  &(cs){.x = 1, .y = 2, .z = 3, .active = true});
    TEST_ASSERT(id_cs_map_has(&tmap, &(id){.is_active = true, .uid = i}));
  }

  /* Check hash algo */
  for (int i = 0; i < 500; i++) {
    TEST_ASSERT(!id_cs_map_has(&tmap, &(id){.is_active = false, .uid = i}));
  }

  for (int i = 0; i < 500; i++) {
    id_cs_map_remove(&tmap, &(id){.is_active = true, .uid = i});
    TEST_ASSERT(!id_cs_map_has(&tmap, &(id){.is_active = true, .uid = i}));
  }

  TEST_ASSERT(tmap.slots_in_use == 0);
}