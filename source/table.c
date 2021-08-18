//
// Created by kyle on 8/18/21.
//

#include <string.h>

#include <clox/memory.h>
#include <clox/object.h>
#include <clox/table.h>

#define TABLE_MAX_LOAD 0.75

void initTable(Table* table) {
  table->count = 0;
  table->capacity = 0;
  table->entries = NULL;
}
void freeTable(Table* table) {
  FREE_ARRAY(Entry, table->entries, table->capacity);
  initTable(table);
}
static Entry* findEntry(Entry* entries, int capacity, ObjString* key) {
  uint32_t index = key->hash % capacity;
  Entry* tombstone = NULL;
  for (;;) {
    Entry* entry = &entries[index];
    if (entry->key == NULL) {
      if (IS_NIL(entry->value)) {
        // no such entry
        return tombstone != NULL ? tombstone : entry;
      } else if (tombstone == NULL) {
        // found tombstone
        tombstone = entry;
      }
    } else if (entry->key == key) {
      // found entry
      return entry;
    }

    index = (index + 1) % capacity;
  }
}
static void adjustCapacity(Table* table, int capacity) {
  Entry* entries = ALLOCATE(Entry, capacity);
  for (int i = 0; i < capacity; ++i) {
    entries[i].key = NULL;
    entries[i].value = NIL_VAL;
  }

  table->count = 0;
  for (int i = 0; i < table->capacity; ++i) {
    Entry* entry = &table->entries[i];
    if (entry->key == NULL) {
      continue;
    }

    Entry* dest = findEntry(entries, capacity, entry->key);
    dest->key = entry->key;
    dest->value = entry->value;
    table->count++;
  }

  FREE_ARRAY(Entry, table->entries, table->capacity);
  table->entries = entries;
  table->capacity = capacity;
}
bool tableSet(Table* table, ObjString* key, Value value) {
  if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
    int capacity = GROW_CAPACITY(table->capacity);
    adjustCapacity(table, capacity);
  }
  Entry* entry = findEntry(table->entries, table->capacity, key);
  bool isNewKey = entry->key == NULL;
  // tombstones don't count as they were previously allocated
  if (isNewKey && IS_NIL(entry->value)) {
    table->count++;
  }

  entry->key = key;
  entry->value = value;
  return isNewKey;
}
void tableAddAll(Table* from, Table* to) {
  for (int i = 0; i < from->capacity; ++i) {
    Entry* entry = &from->entries[i];
    if (entry->key) {
      tableSet(to, entry->key, entry->value);
    }
  }
}
bool tableGet(Table* table, ObjString* key, Value* value) {
  if (table->count == 0) {
    return false;
  }

  Entry* entry = findEntry(table->entries, table->capacity, key);
  if (entry->key == NULL) {
    return false;
  }

  // allow passing NULL to check membership
  if (value) {
    *value = entry->value;
  }
  return true;
}
bool tableDelete(Table* table, ObjString* key) {
  if (table->count == 0) {
    return false;
  }

  Entry* entry = findEntry(table->entries, table->capacity, key);
  if (entry->key == NULL) {
    return false;
  }

  entry->key = NULL;
  // tombstone
  entry->value = BOOL_VAL(true);
  return true;
}
ObjString* tableFindString(
    Table* table,
    int length,
    const char* chars,
    uint32_t hash) {
  if (table->count == 0) {
    return NULL;
  }

  uint32_t index = hash % table->capacity;
  for (;;) {
    Entry* entry = &table->entries[index];
    if (entry->key == NULL) {
      if (IS_NIL(entry->value)) {
        return NULL;
      }
    } else if (
        entry->key->length == length && entry->key->hash == hash
        && memcmp(entry->key->chars, chars, length) == 0) {
      return entry->key;
    }

    index = (index + 1) % table->capacity;
  }
}
