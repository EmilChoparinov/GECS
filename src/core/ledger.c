#include "ledger.h"

retcode ledger_init(ledger *l) {
  gid_l_record_stack_map_init(&l->entity_operations);
  return R_OKAY;
}
void ledger_free(ledger *l) {
  gid_l_record_stack_map_free(&l->entity_operations);
}

retcode ledger_new_entry(ledger *l, gid *entt, l_record *record) {
    assert(l);
    assert(entt);
    assert(record);

    if(!gid_l_record_stack_map_has(&l->entity_operations, entt));

    // l_record_stack stack = 
}