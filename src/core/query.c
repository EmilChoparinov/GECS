#include "gecs.h"

/*-------------------------------------------------------
 * Sequential Query Operations
 *-------------------------------------------------------*/
g_pool gq_seq(g_query *q) {
  g_pool pool = {0};

  pool.entities = gq_vectorize(q);
  pool.idx = 0;

  return pool;
}

g_pool gq_next(g_pool itr) {
  assert(itr.idx < itr.entities.stored_components->length);
  itr.idx++;
  return itr;
}

bool gq_done(g_pool itr) {
  return itr.idx == itr.entities.stored_components->length;
}

void *__gq_field(g_pool *itr, char *type) {
  log_enter;
  gid    type_id = (gid)hash_bytes(type, strlen(type));
  gsize *offset =
      hash_to_size_get(itr->entities.component_offsets, &type_id).value;

  assert(offset && "Entity does not have this component");

  cbuff buff;
  buff_init(&buff, vec_at(itr->entities.stored_components, itr->idx));
  log_leave;
  return buff_skip(&buff, *offset);
}

/*-------------------------------------------------------
 * Parallel Query Operations
 *-------------------------------------------------------*/
g_par gq_vectorize(g_query *q) {
  g_par itr = {0};
  itr.component_offsets = &q->archetype_ctx->offsets;
  itr.stored_components = &q->archetype_ctx->components;
  itr.tick = q->world_ctx->tick;
  return itr;
}

typedef struct __gq_each_args __gq_each_args;
struct __gq_each_args {
  int64_t start_at, stop_at;
  g_par   entities;
  void   *args;
  _each   func;
};
void *__gq_each_thread(void *args) {
  __gq_each_args *input = (__gq_each_args *)args;
  for (int64_t i = input->start_at; i < input->stop_at; i++) {
    input->func(&(g_pool){.idx = i, .entities = input->entities}, input->args);
  }

  return NULL;
}
void __gq_each(g_par vec, _each func, void *args) {
  if (vec.stored_components->length == 0) return;

  /* Split over 8 threads */
  pthread_t      threads[8];
  __gq_each_args thread_args[8];
  int64_t        step = vec.stored_components->length / 8;

  /* For items that are < 8, we spint up a thread for each */
  if (step == 0) step = 1;

  int64_t thread_id = 0;
  int64_t start_idx = 0;

  for (int64_t i = 0; i < 8 && start_idx < vec.stored_components->length; i++) {
    int64_t stop_idx = start_idx + step;
    if (i == 7 || stop_idx >= vec.stored_components->length) {
      stop_idx = vec.stored_components->length;
    }

    thread_args[thread_id] = (__gq_each_args){.start_at = start_idx,
                                              .stop_at = stop_idx,
                                              .entities = vec,
                                              .args = args,
                                              .func = func};

    pthread_create(&threads[thread_id], NULL, __gq_each_thread,
                   &thread_args[thread_id]);
    thread_id++;
    start_idx = stop_idx;
  }
  for (int64_t i = 0; i < thread_id; i++) {
    pthread_join(threads[i], NULL);
  }
}
