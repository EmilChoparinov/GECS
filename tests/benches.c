#include "gecs.h"
#include <omp.h>
#include <time.h>
#include <unistd.h>

#define THREADS_NB omp_get_max_threads()

/* Modified from: https://stackoverflow.com/a/66792679 */
#define bench_block(block)                                                     \
  do {                                                                         \
    clock_t start_time = clock();                                              \
    _Pragma("omp parallel for private(i) num_threads(THREADS_NB)")             \
        block double elapsed_time =                                            \
            (double)(clock() - start_time) / CLOCKS_PER_SEC;                   \
    printf("%f, ", elapsed_time);                                              \
  } while (0)

void setUp() {}
void tearDown() {}

typedef struct CompA CompA;
struct CompA {
  int8_t _;
};

typedef struct CompB CompB;
struct CompB {
  int8_t _;
};


feach(unpack_entity, g_pool, entt, {
    ((CompA*)gq_field(entt, CompA))->_ = 0;
    ((CompB*)gq_field(entt, CompB))->_ = 0;
});
void unpack_comp(g_query *q) {
 g_par elements = gq_vectorize(q);
 gq_each(elements, unpack_entity, NULL);    
}

void bench_read_N_entities_components() {

  int entity_cnt[11] = {1, 4, 8, 16, 32, 64, 256, 1024, 4096, 16000, 0};

  int idx = 0;
  log_set_level(LOG_ERROR);
  while (entity_cnt[idx] != 0) {
    int     cnt = entity_cnt[idx];
    g_core *world = g_create_world();
    G_COMPONENT(world, CompA);
    G_COMPONENT(world, CompB);

    G_SYSTEM(world, unpack_comp, CompA, CompB);

    // printf("bench_destroy_N_entities_with_2_components: %d Entities\n", cnt);
    for (int i = 0; i < cnt; i++) {
      gid entt = g_create_entity(world);
      G_ADD_COMPONENT(world, entt, CompA);
      G_ADD_COMPONENT(world, entt, CompB);
      g_mark_delete(world, entt);
    }
   
    bench_block({
        g_progress(world);
    });

    g_destroy_world(world);
    idx++;
  }
}

void bench_destroy_N_entities_with_2_components() {

  int entity_cnt[11] = {1, 4, 8, 16, 32, 64, 256, 1024, 4096, 16000, 0};

  int idx = 0;
  log_set_level(LOG_ERROR);
  while (entity_cnt[idx] != 0) {
    int     cnt = entity_cnt[idx];
    g_core *world = g_create_world();
    G_COMPONENT(world, CompA);
    G_COMPONENT(world, CompB);

    // printf("bench_destroy_N_entities_with_2_components: %d Entities\n", cnt);
    bench_block({
      for (int i = 0; i < cnt; i++) {
        gid entt = g_create_entity(world);
        G_ADD_COMPONENT(world, entt, CompA);
        G_ADD_COMPONENT(world, entt, CompB);
        g_mark_delete(world, entt);
      }
    });

    g_destroy_world(world);
    idx++;
  }
}

void bench_create_N_entities_with_2_components() {

  int entity_cnt[15] = {1,    4,     8,     16,     32,      64,      256, 1024,
                        4096, 16000, 65000, 262000, 1000000, 2000000, 0};

  int idx = 0;
  log_set_level(LOG_ERROR);
  while (entity_cnt[idx] != 0) {
    int     cnt = entity_cnt[idx];
    g_core *world = g_create_world();
    G_COMPONENT(world, CompA);
    G_COMPONENT(world, CompB);

    printf("bench_create_N_entities_with_2_components: %d Entities\n", cnt);
    bench_block({
      for (int i = 0; i < cnt; i++) {
        gid entt = g_create_entity(world);
        G_ADD_COMPONENT(world, entt, CompA);
        G_ADD_COMPONENT(world, entt, CompB);
      }
    });

    g_destroy_world(world);
    idx++;
  }
}

int main(void) {
  bench_destroy_N_entities_with_2_components();

  return 0;
}