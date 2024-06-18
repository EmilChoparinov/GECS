#include "unity.h"

#include "buffer.h"
#include "vector.h"

typedef struct value value;
struct value {
  int a, b, c;
};

/*-------------------------------------------------------
 * REGISTER
 *-------------------------------------------------------*/
any_vec_t vec;
void setUp(void) { vec_unknown_type_init(&vec, sizeof(char) + sizeof(value)); }
void tearDown(void) { vec_unknown_type_free(&vec); }

void tuple_insert_and_read_tests(void) {
  void *ptr = malloc(sizeof(char) + sizeof(value));

  buff_t buff;
  buff_init(&buff, ptr);

  for (int i = 0; i < 20; i++) {
    char  x = (char)i;
    value local_v = {.a = i, .b = i + 1, .c = i + 2};

    buff_push(&buff, &x, sizeof(char));
    buff_push(&buff, &local_v, sizeof(value));

    void *v = buff_pop(&buff, sizeof(value));
    TEST_ASSERT(memcmp(&local_v, v, sizeof(value)) == 0);
    v = buff_pop(&buff, sizeof(char));
    TEST_ASSERT(memcmp(&x, v, sizeof(char)) == 0);
  }
  free(ptr);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(tuple_insert_and_read_tests);

  UNITY_END();
}
