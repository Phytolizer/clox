//
// Created by kyle on 8/15/21.
//

#include <stdio.h>
#include <string.h>

#include <clox/memory.h>
#include <clox/object.h>
#include <clox/table.h>
#include <clox/value.h>
#include <clox/vm.h>

#define ALLOCATE_OBJ(type, objectType) \
  (type*)allocateObject(sizeof(type), objectType)

extern Vm g_VM;

static Obj* allocateObject(size_t size, ObjType type) {
  Obj* object = (Obj*)reallocate(NULL, 0, size);
  object->type = type;
  object->next = g_VM.objects;
  g_VM.objects = object;
  return object;
}

static ObjString* allocateString(
    int length,
    char chars[length],
    uint32_t hash) {
  ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
  string->length = length;
  string->chars = chars;
  string->hash = hash;
  tableSet(&g_VM.strings, string, NIL_VAL);
  return string;
}

static uint32_t hashString(int length, const char key[length]) {
  uint32_t hash = 2166136261U;
  for (int i = 0; i < length; ++i) {
    hash ^= (uint8_t)key[i];
    hash *= 16777619;
  }
  return hash;
}

ObjString* copyString(int length, const char chars[length]) {
  uint32_t hash = hashString(length, chars);
  ObjString* interned = tableFindString(&g_VM.strings, length, chars, hash);
  if (interned) {
    // no copy necessary :)
    return interned;
  }
  char* heapChars = ALLOCATE(char, length + 1);
  memcpy(heapChars, chars, length);
  heapChars[length] = '\0';
  return allocateString(length, heapChars, hash);
}

void printObject(Value value) {
  switch (OBJ_TYPE(value)) {
    case OBJ_STRING:
      printf("%s", AS_CSTRING(value));
      break;
  }
}

ObjString* takeString(int length, char chars[length]) {
  uint32_t hash = hashString(length, chars);
  ObjString* interned = tableFindString(&g_VM.strings, length, chars, hash);
  if (interned) {
    FREE_ARRAY(char, chars, length + 1);
    return interned;
  }
  return allocateString(length, chars, hash);
}
