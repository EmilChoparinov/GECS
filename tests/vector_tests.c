#include "debug.h"
#include "logger.h"
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

cs_vec_t vector;
void     setUp(void) { cs_vec_init(&vector); }
void     tearDown(void) { cs_vec_free(&vector); }

void push_pop_clear_256(void);
void count_5_circles(void);
void has_random_element(void);

void count_multiples_of_10_to_100(void);
void filter_multiples_of_10_and_sum(void);
void divide_all_even_numbers_by_2_under_100(void);
void add_one_to_odds_and_set_all_evens_to_active(void);

void do_filter_multipies_of_10_and_sum_with_any_type(void);
void push_pop_clear_256_with_any_type(void);

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(push_pop_clear_256);
  RUN_TEST(count_5_circles);
  RUN_TEST(has_random_element);

  RUN_TEST(count_multiples_of_10_to_100);
  RUN_TEST(filter_multiples_of_10_and_sum);
  RUN_TEST(divide_all_even_numbers_by_2_under_100);
  RUN_TEST(add_one_to_odds_and_set_all_evens_to_active);
  RUN_TEST(do_filter_multipies_of_10_and_sum_with_any_type);
  RUN_TEST(push_pop_clear_256_with_any_type);

  UNITY_END();
}

/*-------------------------------------------------------
 * TESTS
 *-------------------------------------------------------*/

void push_pop_clear_256(void) {
  for (int i = 0; i < 256; i++) {
    cs_vec_push(&vector, &(cs){.x = i, .y = i, .z = i, .active = false});
    TEST_ASSERT(cs_vec_top(&vector)->x == i);
  }

  for (int i = 255; i >= 0; i--) {
    TEST_ASSERT(cs_vec_top(&vector)->x == i);
    cs_vec_pop(&vector);
  }

  TEST_ASSERT(vector.length == 0);
  cs_vec_push(&vector, &(cs){.y = 99});
  cs_vec_clear(&vector);
  cs_vec_push(&vector, &(cs){.y = 999});
  TEST_ASSERT(cs_vec_top(&vector)->y == 999);
}
void count_5_circles(void) {
  cs_vec_resize(&vector, 10);
  for (int circle_run = 1; circle_run <= 5; circle_run++) {
    for (int i = 0; i < 10; i++) {
      cs_vec_put(&vector, i, &(cs){.x = circle_run * 10 + i});
      TEST_ASSERT(cs_vec_at(&vector, i)->x = circle_run * 10 + i);
    }
  }
}

bool find_5(cs *a) { return a->x == 5; }
bool find_100(cs *a) { return a->x == 100; }

void has_random_element(void) {
  for (int i = 0; i < 100; i++) {
    cs_vec_push(&vector, &(cs){.x = i});
  }

  cs_vec_t copy;
  cs_vec_copy(cs_vec_init(&copy), &vector);

  cs_vec_filter(&vector, find_5);
  cs_vec_filter(&copy, find_100);

  TEST_ASSERT(vector.length == 1);
  TEST_ASSERT(copy.length == 0);

  cs_vec_free(&copy);
}

bool is_mult_10(cs *a) { return a->x % 10 == 0; }

void count_multiples_of_10_to_100(void) {
  for (int i = 0; i < 100; i++) {
    cs_vec_push(&vector, &(cs){.x = i, .y = i, .z = i, .active = false});
  }

  cs_vec_filter(&vector, is_mult_10);
  TEST_ASSERT(vector.length == 10);
}

cs adder(cs residual, cs *next) {
  residual.x += next->x;
  return residual;
}
void filter_multiples_of_10_and_sum(void) {
  for (int i = 0; i < 100; i++) {
    cs_vec_push(&vector, &(cs){.x = i, .y = i, .z = i, .active = false});
  }

  cs sum =
      cs_vec_foldl(cs_vec_filter(&vector, is_mult_10), adder, (cs){.x = 0});
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
    cs_vec_push(&vector, &(cs){.x = i, .y = i, .z = i, .active = false});
  }

  cs_vec_map(cs_vec_filter(&vector, is_mult_2), divide_2);

  int indexer = 0;
  int counter = 0;
  for (int i = 0; i < 100; i++) {
    if (i % 2 == 0) {
      TEST_ASSERT(cs_vec_at(&vector, indexer)->x == i / 2);
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
    cs_vec_push(&vector, &(cs){.x = i, .y = i, .z = i, .active = false});
  }

  cs_vec_t all_active;
  cs_vec_copy(cs_vec_init(&all_active), &vector);

  cs_vec_filter(cs_vec_map(cs_vec_map(&all_active, to_odd_map), activate),
                is_active);

  TEST_ASSERT(all_active.length == vector.length);
  cs_vec_free(&all_active);
}

void do_filter_multipies_of_10_and_sum_with_any_type(void) {
  any_vec_t container;
  vec_unknown_type_init(&container, sizeof(cs));

  for (int i = 0; i < 100; i++) {
    any_vec_push(&container,
                 (void *)&(cs){.x = i, .y = i, .z = i, .active = false});
  }

  // We cannot fold because the the initial value will be void type. We need to
  // cast and use the correct type to fold!
  cs sum = cs_vec_foldl(cs_vec_filter((cs_vec_t *)&container, is_mult_10),
                        adder, (cs){.x = 0});

  TEST_ASSERT(sum.x == 450);

  any_vec_free(&container);
}

void push_pop_clear_256_with_any_type(void) {
  any_vec_t container;
  vec_unknown_type_init(&container, sizeof(cs));

  for (int i = 0; i < 256; i++) {
    any_vec_push(&container,
                 of_any((&(cs){.x = i, .y = i, .z = i, .active = false})));
    TEST_ASSERT(((cs *)any_vec_top(&container))->x == i);
    TEST_ASSERT(cs_vec_top((cs_vec_t *)&container)->x == i);
  }

  for (int i = 255; i >= 0; i--) {
    TEST_ASSERT(cs_vec_top((cs_vec_t *)&container)->x == i);
    any_vec_pop(&container);
  }

  TEST_ASSERT(container.length == 0);
  any_vec_push(&container, of_any((&(cs){.y = 99})));
  any_vec_clear(&container);
  any_vec_push(&container, of_any((&(cs){.y = 999})));
  TEST_ASSERT(cs_vec_top((cs_vec_t *)&container)->y == 999);

  any_vec_free(&container);
}