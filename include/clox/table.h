//
// Created by kyle on 8/18/21.
//

#ifndef TABLE_H_
#define TABLE_H_

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

void initTable(Table* table);
void freeTable(Table* table);
bool tableGet(Table* table, ObjString* key, Value* value);
bool tableSet(Table* table, ObjString* key, Value value);
bool tableDelete(Table* table, ObjString* key);
void tableAddAll(Table* from, Table* to);
ObjString* tableFindString(
    Table* table,
    int length,
    const char chars[length],
    uint32_t hash);

#endif    // TABLE_H_
