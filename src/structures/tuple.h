#ifndef __HEADER_TUPLE_H__
#define __HEADER_TUPLE_H__

#include <stdlib.h>

typedef struct tuple tuple;

struct tuple {
  void  *e_0;
  void  *e_1;
};

void *tuple_0(tuple *t);
void *tuple_1(tuple *t);

#endif