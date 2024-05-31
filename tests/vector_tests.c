#include "unity.h"
#include "vector.h"

typedef struct complex_struct cs;
struct complex_struct {
  int  x, y, z;
  bool active;
};

VECTOR_GEN_H(cs);
VECTOR_GEN_C(cs);

/*-------------------------------------------------------
 * REGISTER
 *-------------------------------------------------------*/

vec_cs_t vector;
void     setUp(void) { vec_cs_init(&vector); }
void     tearDown(void) { vec_cs_free(&vector); }

void push_pop_clear_256(void);
void count_5_circles(void);
void has_random_element(void);

void count_multiples_of_10_to_100(void);
void filter_multiples_of_10_and_sum(void);
void divide_all_even_numbers_by_2_under_100(void);
void add_one_to_odds_and_set_all_evens_to_active(void);

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(push_pop_clear_256);
  RUN_TEST(count_5_circles);
  RUN_TEST(has_random_element);

  RUN_TEST(count_multiples_of_10_to_100);
  RUN_TEST(filter_multiples_of_10_and_sum);
  RUN_TEST(divide_all_even_numbers_by_2_under_100);
  RUN_TEST(add_one_to_odds_and_set_all_evens_to_active);

  UNITY_END();
}

/*-------------------------------------------------------
 * TESTS
 *-------------------------------------------------------*/

void push_pop_clear_256(void) {
  for (int i = 0; i < 256; i++) {
    vec_cs_push(&vector, &(cs){.x = i, .y = i, .z = i, .active = false});
    TEST_ASSERT(vec_cs_top(&vector)->x == i);
  }

  for (int i = 255; i >= 0; i--) {
    TEST_ASSERT(vec_cs_top(&vector)->x == i);
    vec_cs_pop(&vector);
  }

  TEST_ASSERT(vector.length == 0);
  vec_cs_push(&vector, &(cs){.y = 99});
  vec_cs_clear(&vector);
  vec_cs_push(&vector, &(cs){.y = 999});
  TEST_ASSERT(vec_cs_top(&vector)->y == 999);
}
void count_5_circles(void) {
  vec_cs_resize(&vector, 10);
  for (int circle_run = 1; circle_run <= 5; circle_run++) {
    for (int i = 0; i < 10; i++) {
      vec_cs_put(&vector, i, &(cs){.x = circle_run * 10 + i});
      TEST_ASSERT(vec_cs_at(&vector, i)->x = circle_run * 10 + i);
    }
  }
}

bool find_5(cs *a) { return a->x == 5; }
bool find_100(cs *a) { return a->x == 100; }

void has_random_element(void) {
  for (int i = 0; i < 100; i++) {
    vec_cs_push(&vector, &(cs){.x = i});
  }

  vec_cs_t copy;
  vec_cs_copy(vec_cs_init(&copy), &vector);

  vec_cs_filter(&vector, find_5);
  vec_cs_filter(&copy, find_100);

  TEST_ASSERT(vector.length == 1);
  TEST_ASSERT(copy.length == 0);

  vec_cs_free(&copy);
}

bool is_mult_10(cs *a) { return a->x % 10 == 0; }

void count_multiples_of_10_to_100(void) {
  for (int i = 0; i < 100; i++) {
    vec_cs_push(&vector, &(cs){.x = i, .y = i, .z = i, .active = false});
  }

  vec_cs_filter(&vector, is_mult_10);
  TEST_ASSERT(vector.length == 10);
}

cs adder(cs residual, cs *next) {
  residual.x += next->x;
  return residual;
}
void filter_multiples_of_10_and_sum(void) {
  for (int i = 0; i < 100; i++) {
    vec_cs_push(&vector, &(cs){.x = i, .y = i, .z = i, .active = false});
  }

  cs sum =
      vec_cs_foldl(vec_cs_filter(&vector, is_mult_10), adder, (cs){.x = 0});
  TEST_ASSERT(sum.x == 450);
}

cs divide_2(cs a) {
  if (a.x == 0) return a;
  a.x /= 2;
  return a;
}
bool is_mult_2(cs *a) { return a->x % 2 == 0; }
void divide_all_even_numbers_by_2_under_100(void) {
  for (int i = 0; i < 100; i++) {
    vec_cs_push(&vector, &(cs){.x = i, .y = i, .z = i, .active = false});
  }

  vec_cs_map(vec_cs_filter(&vector, is_mult_2), divide_2);

  int indexer = 0;
  int counter = 0;
  for (int i = 0; i < 100; i++) {
    if (i % 2 == 0) {
      TEST_ASSERT(vec_cs_at(&vector, indexer)->x == i / 2);
      indexer++;
      counter++;
    }
  }
  TEST_ASSERT(vector.length == counter);
}

bool is_active(cs *a) { return a->active; }
cs   to_odd_map(cs a) {
  a.x = a.x % 2 == 0 ? a.x : a.x + 1;
  return a;
}
cs activate(cs a) {
  a.active = true;
  return a;
}
void add_one_to_odds_and_set_all_evens_to_active(void) {
  for (int i = 0; i < 100; i++) {
    vec_cs_push(&vector, &(cs){.x = i, .y = i, .z = i, .active = false});
  }

  vec_cs_t all_active;
  vec_cs_copy(vec_cs_init(&all_active), &vector);

  vec_cs_filter(vec_cs_map(vec_cs_map(&all_active, to_odd_map), activate),
                is_active);

  TEST_ASSERT(all_active.length == vector.length);
  vec_cs_free(&all_active);
}