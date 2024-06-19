#ifndef __HEADER_LEDGER_H__
#define __HEADER_LEDGER_H__

#include "register.h"

enum l_opcodes { L_MOVE_LOSE_COMPONENT, L_MOVE_GAIN_COMPONENT };

typedef struct l_record;
struct l_record {
  l_opcodes code;
  gid       component_id;
  void     *component_data;
};

VECTOR_GEN_H(l_record);
typedef l_record_vec_t l_record_stack;
MAP_GEN_H(gid, l_record_stack);

typedef struct ledger ledger;
struct ledger {
  gid_l_record_stack_map_t entity_operations;
};

retcode ledger_init(ledger *l);
void    ledger_free(ledger *l);
retcode ledger_new_entry(ledger *l, gid *entt, l_record *record);

#endif