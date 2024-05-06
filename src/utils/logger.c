#include "logger.h"

typedef struct log_t log_t;
struct log_t {
  int         LEVEL, LINE_NUM;
  const char *FILE, *FORMAT;
  va_list     to_print;
};

static void log_to_stdout(log_t *log);

int min_level = LOG_TRACE;

void set_level(int LEVEL) { min_level = LEVEL; }

void _cust_log(int LEVEL, const char *FILE, int LINE_NUM, const char *FORMAT,
               ...) {
  if (LEVEL < min_level) return;

  log_t log_instance = {
      .FILE = FILE, .FORMAT = FORMAT, .LEVEL = LEVEL, .LINE_NUM = LINE_NUM};

  va_start(log_instance.to_print, FORMAT);
  log_to_stdout(&log_instance);
  va_end(log_instance.to_print);
}

static void log_to_stdout(log_t *log) {
  char       time_buf[16];
  time_t     time_obj;
  struct tm *loc_time;

  time_obj = time(NULL);
  loc_time = localtime(&time_obj);

  time_buf[strftime(time_buf, sizeof(time_buf), "%H:%M:%S", loc_time)] = '\0';

  printf("%s ", time_buf);

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

  printf("%s:%d: ", log->FILE, log->LINE_NUM);
  vfprintf(stdout, log->FORMAT, log->to_print);
  fflush(stdout);
}