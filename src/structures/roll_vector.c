#include "roll_vector.h"

struct roll_vec_t {
  short_vec_t **chunks;
  short_vec_t  *lookup;
};

roll_vec_t *roll_vec_make(size_t el_size, size_t chunk_size, int chunk_count) {
  roll_vec_t *vec;
  int         itr;
  if ((vec = malloc(sizeof(*vec))) == NULL) return NULL;

  if ((vec->chunks = malloc(sizeof(*vec->chunks) * chunk_count)) == NULL)
    return NULL;

  for (itr = 0; itr < chunk_count; itr++) {
    vec->chunks[itr] = short_vec_make(el_size, chunk_size);
    if (vec->chunks[itr] == NULL) return NULL;
  }

  if ((vec->lookup = short_vec_make(sizeof(void *), chunk_count)) == NULL)
    return NULL;

  return vec;
}

int roll_vec_free(roll_vec_t *v) {
  int itr;
  short_vec_free(v->lookup);
  for (itr = 0; itr < short_vec_len(v->lookup); itr++)
    short_vec_free(short_vec_at(v->lookup, itr));
  free(v->chunks);
  return ROLL_VEC_OK;
}

void *roll_vec_at(roll_vec_t *v, int i);
int   roll_vec_push(roll_vec_t *v, void *el);
void *roll_vec_pop(roll_vec_t *v);
void *roll_vec_first(roll_vec_t *v);
void *roll_vec_top(roll_vec_t *v);

int    roll_vec_len(roll_vec_t *v);
size_t roll_vec_sizeof(void);