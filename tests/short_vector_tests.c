#include "short_vector.h"
#include "unity.h"

//== REGISTER ==================================================================

short_vec_t *test_vector;

void setUp(void) { test_vector = short_vec_make(sizeof(int), 2); }

void tearDown(void) { short_vec_free(test_vector); }

void create_and_delete(void);

void push_256(void);

void push_and_pop_256_times(void);

void fill_256(void);

void initial_state_check(void);

void first_last_check(void);

void has_check(void);

void has_no_check(void);

void find_check(void);

void no_find_check(void);

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(create_and_delete);
  RUN_TEST(push_256);
  RUN_TEST(push_and_pop_256_times);
  RUN_TEST(fill_256);
  RUN_TEST(initial_state_check);
  RUN_TEST(first_last_check);
  RUN_TEST(has_check);
  RUN_TEST(has_no_check);
  RUN_TEST(find_check);
  RUN_TEST(no_find_check);

  return UNITY_END();
}

//== TESTS =====================================================================
void create_and_delete(void) {
  short_vec_t *test = short_vec_make(sizeof(int), 256);
  TEST_ASSERT(test != NULL);
  TEST_ASSERT(short_vec_free(test) == SHORT_VEC_OK);
}

void push_256(void) {
  for (int i = 0; i < 256; i++) {
    short_vec_push(test_vector, &i);
    TEST_ASSERT(*(int *)short_vec_top(test_vector) == i);
  }
}

void push_and_pop_256_times(void) {
  for (int i = 0; i < 256; i++) {
    short_vec_push(test_vector, &i);
    TEST_ASSERT(*(int *)short_vec_top(test_vector) ==
                *(int *)short_vec_pop(test_vector));
  }
  TEST_ASSERT(short_vec_len(test_vector) == 0);
}

void fill_256(void) {
  short_vec_t *test = short_vec_make(sizeof(int), 256);
  for (int i = 0; i < 256; i++) {
    short_vec_push(test, &i);
  }

  for (int i = 0; i < 256; i++) {
    TEST_ASSERT(*(int *)short_vec_at(test, i) == i);
  }
  short_vec_free(test);
}

void initial_state_check(void) {
  TEST_ASSERT(short_vec_first(test_vector) == NULL);
  TEST_ASSERT(short_vec_top(test_vector) == NULL);
  TEST_ASSERT(short_vec_pop(test_vector) == NULL);
  TEST_ASSERT(short_vec_at(test_vector, 1000) == NULL);
  TEST_ASSERT(short_vec_at(test_vector, short_vec_len(test_vector)) == NULL);
}

void first_last_check(void) {
  int x = 5;
  short_vec_push(test_vector, &x);
  TEST_ASSERT(*(int *)short_vec_first(test_vector) ==
              *(int *)short_vec_top(test_vector));
}

void has_check(void) {
  int a, b, c;

  a = 5;
  b = 6;
  c = 7;

  short_vec_push(test_vector, &a);
  short_vec_push(test_vector, &b);
  short_vec_push(test_vector, &c);

  TEST_ASSERT(short_vec_has(test_vector, &a));
  TEST_ASSERT(short_vec_has(test_vector, &b));
  TEST_ASSERT(short_vec_has(test_vector, &c));
}

void has_no_check(void) {
  int a, b, c;

  a = 5;
  b = 6;
  c = 7;

  short_vec_push(test_vector, &a);

  TEST_ASSERT(short_vec_has(test_vector, &a));
  TEST_ASSERT(!short_vec_has(test_vector, &b));
  TEST_ASSERT(!short_vec_has(test_vector, &c));
}

bool look_for_6(const void *el) { return *(int *)el == 6; }
bool look_for_5(const void *el) { return *(int *)el == 5; }

void find_check(void) {
  int a, b, c;

  a = 5;
  b = 6;
  c = 7;

  short_vec_push(test_vector, &a);
  short_vec_push(test_vector, &b);
  short_vec_push(test_vector, &c);

  TEST_ASSERT(short_vec_find(test_vector, look_for_6) != NULL);
}

void no_find_check(void) {
  int a;

  a = 5;

  short_vec_push(test_vector, &a);

  TEST_ASSERT(short_vec_find(test_vector, look_for_6) == NULL);
  TEST_ASSERT(short_vec_find(test_vector, look_for_5) != NULL);
}