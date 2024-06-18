#include "str_utils.h"

char *fix_str_256(char *str) {
  size_t str_len = 0;
  while (str[str_len] != 0 && str_len < 256) str_len++;

  assert(str_len != 0 && "String cannot be empty!");
  assert(str_len != 256 && "String was longer than 256!");

  char *_str = calloc(1, 256);
  memmove(_str, str, str_len);

  return _str;
}