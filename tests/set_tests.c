#include "logger.h"
#include "set.h"
#include "unity.h"

typedef struct complex_struct cs;
struct complex_struct {
  int  x, y, z;
  bool active;
} __attribute__((packed));

SET_GEN_H(cs);
SET_GEN_C(cs);

/*-------------------------------------------------------
 * REGISTER
 *-------------------------------------------------------*/
cs_set_t cont;
void     setUp(void) { cs_set_init(&cont); }
void     tearDown(void) { cs_set_free(&cont); }

void test_insert(void);
void test_remove(void);
void test_fill(void);

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_insert);
  RUN_TEST(test_remove);
  RUN_TEST(test_fill);

  UNITY_END();
}

/*-------------------------------------------------------
 * TESTS
 *-------------------------------------------------------*/
void test_insert(void) {
  TEST_ASSERT(
      !cs_set_has(&cont, &(cs){.active = true, .x = 69, .y = 69, .z = 69}));

  TEST_ASSERT(
      cs_set_place(&cont, &(cs){.active = true, .x = 69, .y = 69, .z = 69}));

  TEST_ASSERT(
      !cs_set_place(&cont, &(cs){.active = true, .x = 69, .y = 69, .z = 69}));
}
void test_remove(void) {
  test_insert();

  TEST_ASSERT(
      cs_set_delete(&cont, &(cs){.active = true, .x = 69, .y = 69, .z = 69}));
  TEST_ASSERT(
      !cs_set_delete(&cont, &(cs){.active = true, .x = 69, .y = 69, .z = 69}));
}

void test_fill(void) {
  for (int i = 0; i < 500; i++) {
    TEST_ASSERT(
        cs_set_place(&cont, &(cs){.active = true, .x = i, .y = i, .z = i}));

    /* No duplicates */
    TEST_ASSERT(
        !cs_set_place(&cont, &(cs){.active = true, .x = i, .y = i, .z = i}));

    /* Hashing works */
    TEST_ASSERT(
        cs_set_has(&cont, &(cs){.active = true, .x = i, .y = i, .z = i}));
    TEST_ASSERT(
        !cs_set_has(&cont, &(cs){.active = false, .x = i, .y = i, .z = i}));
  }

  TEST_ASSERT(cs_set_count(&cont) == 500);

  for (int i = 0; i < 500; i++) {
    TEST_ASSERT(
        cs_set_delete(&cont, &(cs){.active = true, .x = i, .y = i, .z = i}));
    TEST_ASSERT(
        !cs_set_has(&cont, &(cs){.active = true, .x = i, .y = i, .z = i}));
  }

  TEST_ASSERT(cs_set_count(&cont) == 0);
}