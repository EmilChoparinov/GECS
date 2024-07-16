#include "gecs.h"
#include "unity.h"

void setUp() {}
void tearDown() {}

/*-------------------------------------------------------
 * TESTS
 *-------------------------------------------------------*/
typedef struct Vec2 Vec2;
struct Vec2 {
  int32_t x, y;
};

void measure_entity_allocs() {
  g_core *world = g_create_world();

  //   G_COMPONENT(world, Vec2);
  g_create_entity(world);
//   g_create_entity(world);
//   g_create_entity(world);
//   g_create_entity(world);
//   g_create_entity(world);
//   g_create_entity(world);

  g_destroy_world(world);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(measure_entity_allocs);

  UNITY_END();
  return 0;
}