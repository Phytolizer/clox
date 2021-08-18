//
// Created by kyle on 8/18/21.
//

#ifndef TABLE_H_
#define TABLE_H_

#include "attributes.h"
#include "common.h"
#include "value.h"

typedef struct entry_s {
  ObjString* key;
  Value value;
} Entry;

typedef struct table_s {
  int count;
  int capacity;
  Entry* entries;
} Table;

void initTable(Table* table) ATTR_NONNULL(1);
void freeTable(Table* table) ATTR_NONNULL(1);
bool tableGet(Table* table, ObjString* key, Value* value) ATTR_NONNULL(1, 2);
bool tableSet(Table* table, ObjString* key, Value value) ATTR_NONNULL(1, 2);
bool tableDelete(Table* table, ObjString* key) ATTR_NONNULL(1, 2);
void tableAddAll(Table* from, Table* to) ATTR_NONNULL(1, 2);
ObjString* tableFindString(
    Table* table,
    int length,
    const char chars[length],
    uint32_t hash) ATTR_NONNULL(1);

#endif    // TABLE_H_
