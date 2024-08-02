#include "gecs.h"
#include "archetype.h"
#include "component.h"
#include "entity.h"
#include "gid.h"
#include <stdio.h>

/*-------------------------------------------------------
 * Static Functions
 *-------------------------------------------------------*/
static void archetype_simulate_deletions(g_core *w, archetype *a) {
  /* Deletion Algorithm:
       - For each entity in the delete buffer
       - Pop
       - Delete entity links in the registry
       - Delete entity links in the simulation registry if exists */
  while (a->entt_deletion_buffer.length) {
    gid *entt = id_vec_top(&a->entt_deletion_buffer);
    id_vec_pop(&a->entt_deletion_buffer);

    id_to_hash_del(&w->entity_registry, entt);
    id_to_int64_del(&a->entt_positions, entt);
    id_to_hash_del(&a->simulation->entity_registry, entt);
  }
}

static void archetype_simulate_creations(g_core *w, archetype *a) {
  /* Creation Algorithm:
       - For each entity in the creation buffer
       - Pop
       - Load the entity archetype inside the simulation
       - Perform component data transition from the simulation to
       - the real context */

  while (a->entt_creation_buffer.length) {
    gid *entt = id_vec_top(&a->entt_creation_buffer);
    id_vec_pop(&a->entt_creation_buffer);

    /* This check is to ensure that the entity was not deleted by the simulate
       deletion algorithm since it runs first. */
    if (id_to_hash_has(&w->entity_registry, entt)) continue;

    /* Offically add the entity to the map */
    map_put(&w->entity_registry, entt, &empty_archetype.hash_name);
    G_ADD_COMPONENT(w, *entt, GecID);
    G_SET_COMPONENT(w, *entt, GecID, {.id = *entt});

    /* Load simulated archetype */
    archetype *sim_arch = load_entity_archetype(a->simulation, *entt);
    hash_vec   types;
    set_to_vec(&sim_arch->types, (hash_vec *)&types);
    _g_add_component(w, *entt, &types);

    /* Perform component data transition */
    // TODO: This is an optimization opportunity. It's possible to directly
    //       do a memmove instead of using get_component and set_component
    for (int64_t type_i = 0; type_i < types.length; type_i++) {
      gid *type = hash_vec_at(&types, type_i);
      if (_g_has_component(a->simulation, *entt, *type)) {
        void *seg = _g_get_component(a->simulation, *entt, *type);
        _g_set_component(w, *entt, *type, seg);
      }
    }
  }
}

static void entity_simulate_component_operations(g_core *w, archetype *a) {
  /* Entity Consolidation Algorithm:
        - For each entity in the mutation buffer
        - Pop
        - Load both simulated and real archetypes
        - Take the type difference
        - Move types in the difference to the real context */
  while (a->entt_mutation_buffer.length) {
    gid *entt = id_vec_top(&a->entt_mutation_buffer);
    id_vec_pop(&a->entt_mutation_buffer);

    /* This check is to ensure that the entity was not deleted by the simulate
    deletion algorithm since it runs first. */
    if (id_to_hash_has(&w->entity_registry, entt)) continue;

    /* Load archetypes */
    archetype *real_arch = load_entity_archetype(w, *entt);
    archetype *sim_arch = load_entity_archetype(a->simulation, *entt);

    /* Transition to where the tick simulated the entity will go */
    hash_vec types;
    set_to_vec(&sim_arch->types, (hash_vec *)&types);
    delta_transition(w, *entt, &types);

    /* All data in the real context is more fresh than the data in the
       simulated context. So we only move the *new* component data from
       simulated to real. */
    // TODO: This is also an optimization opportunity for the same reason as
    //       in `archetype_simulate_creations`
    for (int64_t type_i = 0; type_i < types.length; type_i++) {
      gid *type = hash_vec_at(&types, type_i);

      /* Non simulated components are preferred. */
      if (type_set_has(&real_arch->types, type)) continue;

      void *new_seg = _g_get_component(a->simulation, *entt, *type);
      _g_set_component(w, *entt, *type, new_seg);
    }
  }
}

feach(migrate_archetype, kvpair, item, {
  g_core *w = (g_core *)args;
  archetype_simulate_deletions(w, item.value);
  archetype_simulate_creations(w, item.value);
  entity_simulate_component_operations(w, item.value);
});
static void migration_routine(g_core *w) {
  log_enter;

  /* Entity FSM migration to real context routine. */
  hash_to_archetype_foreach(&w->archetype_registry, migrate_archetype, w);

  log_leave;
}

feach(reset_archetype, kvpair, item, {
  archetype *arch = item.value;
  composite_clear(&arch->components);
  id_to_int64_clear(&arch->entt_positions);
});
feach(cleanup_archetype, kvpair, item, {
  archetype *arch = item.value;

  id_vec_clear(&arch->entt_creation_buffer);
  id_vec_clear(&arch->entt_deletion_buffer);
  id_vec_clear(&arch->entt_mutation_buffer);
  id_to_hash_clear(&arch->simulation->entity_registry);
  hash_to_archetype_foreach(&arch->simulation->archetype_registry,
                            reset_archetype, NULL);
});
static void cleanup_routine(g_core *w) {
  hash_to_archetype_foreach(&w->archetype_registry, cleanup_archetype, w);
}

feach(defrag_entity, kvpair, item, {
  int64_vec *rolling_offsets = (int64_vec *)args;
  int64_t   *pos = item.value;
  *pos -= *int64_vec_at(rolling_offsets, *pos);
});

feach(defrag_archetype, kvpair, item, {
  archetype *arch = item.value;
  if (arch->components.length == 0) return;

  composite vectorizor;
  __vec_init(&vectorizor, arch->components.__el_size, arch->allocator, TO_HEAP,
             arch->components.length);

  int64_vec rolling_offsets;
  int64_vec_sinit(&rolling_offsets, arch->components.length);
  int64_t dead_index = 0;
  for (int64_t i = 0; i < arch->components.length; i++) {
    if (dead_index < arch->dead_fragment_buffer.length &&
        *int64_vec_at(&arch->dead_fragment_buffer, dead_index) == i) {
      dead_index++;
      int64_vec_push(&rolling_offsets, &dead_index);
      continue;
    }
    void *seg = composite_at(&arch->components, i);
    composite_push(&vectorizor, seg);
    int64_vec_push(&rolling_offsets, &dead_index);
  }

  /* Rewire vectorizer and components. Since vectorizer is stack allocated,
     we need to recycle the composite address pointer and memove the cleaned
     dataset from vectorizer to components to it. */
  memmove(arch->components.elements, vectorizor.elements,
          vectorizor.length * vectorizor.__el_size);
  void *old_elements = vectorizor.elements;

  vectorizor.elements = arch->components.elements;
  memmove(&arch->components, &vectorizor, sizeof(composite));

  vectorizor.elements = old_elements;
  composite_free(&vectorizor);

  id_to_int64_foreach(&arch->entt_positions, defrag_entity, &rolling_offsets);
});

static void defragment_routine(g_core *w) {
  hash_to_archetype_foreach(&w->archetype_registry, defrag_archetype, NULL);
}

feach(process_archetype_fsm, kvpair, archetype_item, {
  archetype *arch = archetype_item.value;
  g_core    *w = (g_core *)args;

  /* Clear the old contenders from the list */
  system_vec_clear(&arch->contenders);

  /* Iterate over each system and apply each system that intersects on
     their type sets. */
  for (int64_t i = 0; i < w->system_registry.length; i++) {
    system_data *sys = system_vec_at(&w->system_registry, i);
    if (set_is_subset(&arch->types, &sys->requirements)) {
      system_vec_push(&arch->contenders, sys);
    }
  }
});
static void reassign_entity_fsm(g_core *w) {
  log_enter;
  hash_to_archetype_foreach(&w->archetype_registry, process_archetype_fsm, w);
  /* Set process flag as complete so we don't re-process. */
  w->invalidate_fsm = 0;
  log_leave;
}

/*-------------------------------------------------------
 * Container Operations
 *-------------------------------------------------------*/
g_core *g_create_world(void) {
  log_enter;

  g_core *w = calloc(1, sizeof(*w));

  atomic_init(&w->id_gen, 0);
  gid_atomic_set(&w->id_gen, STORAGE);

  w->invalidate_fsm = 1;
  w->is_sequential = 1;
  w->disable_concurrency = 0;
  w->tick = 0;

  w->allocator = stalloc_create(STALLOC_DEFAULT);

  hash_to_archetype_inita(&w->archetype_registry, w->allocator, TO_HEAP,
                          ARCHETYPE_REG_START);
  hash_to_size_inita(&w->component_registry, w->allocator, TO_HEAP,
                     COMPONENT_REG_START);
  id_to_hash_inita(&w->entity_registry, w->allocator, TO_HEAP,
                   ENTITY_REG_START);
  system_vec_inita(&w->system_registry, w->allocator, TO_HEAP,
                   SYSTEM_REG_START);

  /* Default component registrations */
  G_COMPONENT(w, GecID);

  /* I decided to not put the empty archetype into the registry because I do
     not want to risk adding an extra collision when querying. It's ok to not
     put it in because the empty archetype represents garbage, so a user
     cannot query for it. */
  log_leave;
  return w;
}

void *boot_system(void *arg) {
  void       **args = (void **)arg;
  system_data *sys = args[0];
  g_query     *query = args[1];
  sys->start_system(query);
  return NULL;
}

void process_archetype(g_core *w, archetype *process_arch) {
  start_frame(process_arch->allocator);

  system_vec readonlys;
  system_vec_sinit(&readonlys, process_arch->contenders.length);

  for (int64_t i = 0; i < process_arch->contenders.length; i++) {
    system_data *sys = system_vec_at(&process_arch->contenders, i);
    if (sys->readonly != 0) {
      system_vec_push(&readonlys, sys);
      continue;
    }
    sys->start_system(
        &(g_query){.world_ctx = w, .archetype_ctx = process_arch});
  }

  if (readonlys.length == 0) return;

  pthread_t *threads = stpush(readonlys.length * sizeof(pthread_t));

  for (int64_t i = 0; i < readonlys.length; i++) {
    system_data *sys = system_vec_at(&readonlys, i);
    g_query      q = {.world_ctx = w, .archetype_ctx = process_arch};
    if (w->disable_concurrency == 1) {
      sys->start_system(&q);
    } else {
      void **args = stpush(2 * sizeof(void *));
      args[0] = sys;
      args[1] = &q;
      pthread_create(&threads[i], NULL, boot_system, args);
    }
  }

  if (w->disable_concurrency == 0) {
    for (int64_t i = 0; i < readonlys.length; i++) {
      if (pthread_join(threads[i], NULL)) {
        log_debug("Thread unable to be joined");
        exit(EXIT_FAILURE);
      }
    }
  }

  end_frame(process_arch->allocator);
}
void *thread_process_archetype(void *arg) {
  log_enter;
  void **args = (void **)arg;
  process_archetype(args[0], args[1]);
  log_leave;
  return NULL;
}

typedef struct thread_args {
  int64_t    index;
  pthread_t *threads;
  g_core    *w;
} thread_args;
void thread_archetype(void *_el, void *args) {
  kvpair item = *(kvpair *)_el;
  {
    thread_args *targs = (thread_args *)args;
    void       **process_args = __stpushframe(2 * sizeof(void *));
    process_args[0] = targs->w;
    process_args[1] = item.value;
    if (targs->w->disable_concurrency == 1) {
      process_archetype(targs->w, item.value);
      return;
    }
    pthread_create(&targs->threads[targs->index], ((void *)0),
                   thread_process_archetype, process_args);
    targs->index++;
  }
}

// feach(thread_archetype, kvpair, item, {
//   thread_args *targs = (thread_args *)args;

//   void **process_args = stpush(2 * sizeof(void *));
//   process_args[0] = targs->w;
//   process_args[1] = item.value;

//   if (targs->w->disable_concurrency == 1) {
//     process_archetype(targs->w, item.value);
//     return;
//   }

//   pthread_create(&targs->threads[targs->index], NULL,
//   thread_process_archetype,
//                  process_args);
//   targs->index++;
// });
void g_progress(g_core *w) {
  start_frame(w->allocator);
  log_enter;
  log_debug("TICK START");

  w->tick++;
  if (w->invalidate_fsm == 1) reassign_entity_fsm(w);

  int64_t     arch_len = map_load(&w->archetype_registry);
  thread_args args = {
      .index = 0, .threads = stpush(arch_len * sizeof(pthread_t)), .w = w};
  map_foreach(&w->archetype_registry, thread_archetype, &args);

  if (w->disable_concurrency == 0) {
    for (int64_t i = 0; i < arch_len; i++) {
      if (pthread_join(args.threads[i], NULL)) {
        log_debug("Thread unable to be joined");
        exit(EXIT_FAILURE);
      }
    }
  }

  migration_routine(w);
  cleanup_routine(w);
  defragment_routine(w);

  log_debug("TICK END");
  log_leave;
  end_frame(w->allocator);
}

feach(f_free_archetype, kvpair, item, { free_archetype(item.value); });
feach(f_free_system, system_data, sys, { type_set_free(&sys.requirements); });
void g_destroy_world(g_core *w) {
  log_enter;

  hash_to_archetype_foreach(&w->archetype_registry, f_free_archetype, NULL);
  hash_to_archetype_free(&w->archetype_registry);

  id_to_size_free(&w->component_registry);

  system_vec_foreach(&w->system_registry, f_free_system, NULL);
  system_vec_free(&w->system_registry);

  id_to_hash_free(&w->entity_registry);

  stalloc_free(w->allocator);
  free(w);

  log_leave;
}

/*-------------------------------------------------------
 * Thread Unsafe Registration Operations
 *-------------------------------------------------------*/
void g_register_component(g_core *w, char *name, size_t component_size) {
  log_enter;
  start_frame(w->allocator);

  uint64_t hash_name = hash_bytes(name, strlen(name));

  assert(!hash_to_size_has(&w->component_registry, name) &&
         "Collision detection: name is either re-registered or another "
         "component contains the same hashname. Exiting");

  hash_to_size_put(&w->component_registry, &hash_name, &component_size);

  log_debug("registerd new component: %s -> %ld", name, hash_name);

  end_frame(w->allocator);
  log_leave;
}

static feach(add_to_set, uint64_t, hash, {
  type_set *types = args;
  type_set_put(types, &hash);
});

static feach(is_registered, uint64_t, hash, {
  g_core *w = args;
  assert(hash_to_size_get(&w->component_registry, &hash).value &&
         "Error: attempted to register a system with unregistered component "
         "types.");
});
void g_register_system(g_core *w, g_system sys, int32_t FLAGS, char *query) {
  log_enter;
  start_frame(w->allocator);

  /* Collect the component id hashes */
  hash_vec type_hashes;
  archetype_key(query, &type_hashes);
  hash_vec_foreach(&type_hashes, is_registered, w); /* Sanity check */

  type_set types;
  type_set_hinit(&types);
  hash_vec_foreach(&type_hashes, add_to_set, &types);

  system_vec_push(&w->system_registry, &(system_data){.requirements = types,
                                                      .start_system = sys,
                                                      .readonly = FLAGS});

  end_frame(w->allocator);
  log_leave;
}
