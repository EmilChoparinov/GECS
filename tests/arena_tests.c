#include "arena.h"
#include "unity.h"

/*-------------------------------------------------------
 * REGISTER
 *-------------------------------------------------------*/
void setUp(void) {}
void tearDown(void) {}

void test_creation_free(void);
void test_full_alloc(void);
void test_poll_alloc(void);

int main(void) {
  UNITY_BEGIN();

  // RUN_TEST(test_creation_free);
  // RUN_TEST(test_full_alloc);
  // RUN_TEST(test_poll_alloc);

  UNITY_END();
}

/*-------------------------------------------------------
 * TESTS
 *-------------------------------------------------------*/
void test_creation_free(void) { arena_free(arena_init()); }
void test_full_alloc(void) {
  arena_t *ar = arena_init();

  /* Test to see if the whole region is used when alloc'ing */
  TEST_ASSERT(arena_poll(ar) == REGION_SIZE);
  arena_alloc(ar, REGION_SIZE);
  TEST_ASSERT(arena_poll(ar) == 0);

  /* Test to see if we can add regions higher than the REGION_SIZE and the
     memory given of the expected size. We:
        - Allocate a region that is REGION_SIZE + 1
        - Test to see if we added that empty region correctly
        - Allocate REGION_SIZE bytes
        - Test to see if the 1 byte is left
        - Allocate the 1 byte
        - Test to see if the region is now empty */
  arena_push_region(ar, REGION_SIZE + 1);
  TEST_ASSERT(arena_poll(ar) == REGION_SIZE + 1);
  arena_alloc(ar, REGION_SIZE);
  TEST_ASSERT(arena_poll(ar) == 1);
  arena_alloc(ar, 1);
  TEST_ASSERT(arena_poll(ar) == 0);

  /* Test to see if allocating from 0 will break anything, or allocating at
     strange intervals. */
  arena_alloc(ar, 1);
  TEST_ASSERT(arena_poll(ar) == REGION_SIZE - 1);
  arena_alloc(ar, REGION_SIZE - 2);
  TEST_ASSERT(arena_poll(ar) == 1);

  arena_free(ar);
}
void test_poll_alloc(void) {
  arena_t *ar = arena_init();

  int total = REGION_SIZE;
  int sum = 0;
  for (int i = 0; i < total; i += sizeof(int)) {
    int poll1 = arena_poll(ar);

    int *x = arena_alloc(ar, sizeof(int));
    *x = i;
    sum += *x;
    int poll2 = arena_poll(ar);

    TEST_ASSERT(poll1 > poll2);
  }

  printf("sum: %d\n", sum);
  arena_free(ar);
}
