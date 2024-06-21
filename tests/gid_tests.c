#include "gid.h"
#include "unity.h"

void tearDown() {}
void setUp() {}

void test_increment(void) {
  gid id_gen = 0;
  // all 0s to having a 1 at the 33rd place: so we do
  gid expected = 1;
  GID_INCR(id_gen);

  TEST_ASSERT(SELECT_ID(id_gen) == expected);

  expected = 2;
  GID_INCR(id_gen);

  TEST_ASSERT(SELECT_ID(id_gen) == expected);
}

void test_modes(void) {
  gid id_gen = 0;

  GID_SET_MODE(id_gen, STORAGE);

  TEST_ASSERT(SELECT_MODE(id_gen) == STORAGE);

  GID_SET_MODE(id_gen, CACHED);

  TEST_ASSERT(SELECT_MODE(id_gen) == CACHED);
}

void compound_test(void) {
  gid id_gen = 0;

  // generate 20 ids and check
  for (gid expected = 1; expected < 20; expected++) {
    GID_INCR(id_gen);
    TEST_ASSERT(SELECT_ID(id_gen) == expected);
  }

  // change modes
  GID_SET_MODE(id_gen, CACHED);

  for (gid expected = 20; expected < 50; expected++) {
    GID_INCR(id_gen);
    TEST_ASSERT(SELECT_ID(id_gen) == expected);
    TEST_ASSERT(SELECT_MODE(id_gen) == CACHED);
  }
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_increment);
  RUN_TEST(test_modes);
  RUN_TEST(compound_test);

  UNITY_END();
}