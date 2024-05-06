#include "logger.h"
#include "unity.h"

/*-------------------------------------------------------
 * REGISTER
 *-------------------------------------------------------*/
void setUp(void) {}
void tearDown(void) {}

void test_colors(void);

void test_levels(void);

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_colors);
  RUN_TEST(test_levels);

  UNITY_END();
}

/*-------------------------------------------------------
 * TESTS
 *-------------------------------------------------------*/
void test_colors(void) {
  log_trace("trace log colors\n");
  log_info("info log colors\n");
  log_debug("debug log colors\n");
  log_warn("warn log colors\n");
  log_error("error log colors\n");
}

void test_levels(void) {
  log_set_level(LOG_ERROR);
  log_trace("ignored\n");
  log_info("ignored\n");
  log_debug("ignored\n");
  log_warn("ignored\n");
  log_error("only error logs should be active!\n");
}