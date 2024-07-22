#include "logger.h"

#ifdef _DEBUG
/*-------------------------------------------------------
 * TYPEDEFS AND STATIC FUNCTIONS
 *-------------------------------------------------------
 * log_t         - Private structure defined for storing all information
 *                 required print to the target.
 * log_to_stdout - Static function that is used to log directly to stdout.
 *                 Flushes each time (NOT THREAD SAFE!!!!!) */

typedef struct log_t log_t;
struct log_t {
  int         LEVEL, LINE_NUM;
  const char *FILE, *FORMAT, *FUNC;
  va_list     to_print;
};

static void log_to_stdout(log_t *log);

/*-------------------------------------------------------
 * GLOBAL VARIABLES
 *-------------------------------------------------------
 * min_level - variable that should be set once in main. All levels under
 *             min_level will be ignored. */
int min_level = LOG_TRACE;

/*-------------------------------------------------------
 * LIBRARY IMPLEMENTATION
 *-------------------------------------------------------*/
void log_set_level(int LEVEL) { min_level = LEVEL; }

void _cust_log(int LEVEL, const char *FILE, const char *FUNC, int LINE_NUM,
               const char *FORMAT, ...) {
  if (LEVEL < min_level) return;

  log_t log_instance = {.FILE = FILE,
                        .FORMAT = FORMAT,
                        .FUNC = FUNC,
                        .LEVEL = LEVEL,
                        .LINE_NUM = LINE_NUM};

  va_start(log_instance.to_print, FORMAT);
  log_to_stdout(&log_instance);
  va_end(log_instance.to_print);
}

/*-------------------------------------------------------
 * STATIC FUNCTION IMPLEMENTATION
 *-------------------------------------------------------*/
static void log_to_stdout(log_t *log) {
  // char       time_buf[16];
  // time_t     time_obj;
  // struct tm *loc_time;

  // time_obj = time(NULL);
  // loc_time = localtime(&time_obj);


  // time_buf[strftime(time_buf, sizeof(time_buf), "%H:%M:%S", loc_time)] = '\0';

  // printf("%s ", time_buf);

  switch (log->LEVEL) {
  case LOG_TRACE:
    printf(GRN "TRACE " RST);
    break;
  case LOG_INFO:
    printf(BLU "INFO " RST);
    break;
  case LOG_DEBUG:
    printf(MAG "DEBUG " RST);
    break;
  case LOG_WARN:
    printf(YEL "WARN " RST);
    break;
  case LOG_ERROR:
    printf(RED "ERROR " RST);
    break;
  }

  printf("%s:%d [%s]: ", log->FILE, log->LINE_NUM, log->FUNC);
  vfprintf(stdout, log->FORMAT, log->to_print);
  printf("\n");
  fflush(stdout);
}
#endif
