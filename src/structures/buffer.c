#include "buffer.h"

/*-------------------------------------------------------
 * Container Operations
 *-------------------------------------------------------*/
buff_t *buff_init(buff_t *b, void *region) {
  b->region = b->top = region;
  b->len = 0;
  return b;
}
buff_t *buff_hinit(void *region) {
  buff_t *b = malloc(sizeof(buff_t));
  assert(b);

  buff_init(b, region);
  return b;
}

void buff_hfree(buff_t *b) { free(b); }

/*-------------------------------------------------------
 * Element Operations
 *-------------------------------------------------------*/
retcode buff_push(buff_t *b, void *item, size_t item_size) {
  assert(b);
  assert(item);
  assert(item_size > 0);

  b->len++;
  memmove(b->top, item, item_size);
  b->top += item_size;
  return R_OKAY;
}

void *buff_pop(buff_t *b, size_t item_size) {
  assert(b);
  assert(item_size > 0);

  b->len--;
  b->top -= item_size;
  return b->top;
}
