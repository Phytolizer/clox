//
// Created by kyle on 8/15/21.
//

#include <stdio.h>
#include <string.h>

#include <clox/memory.h>
#include <clox/object.h>
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

ObjString* allocateString(int length, char chars[length]) {
  ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
  string->length = length;
  string->chars = chars;
  return string;
}
ObjString* copyString(int length, const char chars[length]) {
  char* heapChars = ALLOCATE(char, length + 1);
  memcpy(heapChars, chars, length);
  heapChars[length] = '\0';
  return allocateString(length, heapChars);
}

void printObject(Value value) {
  switch (OBJ_TYPE(value)) {
    case OBJ_STRING:
      printf("%s", AS_CSTRING(value));
      break;
  }
}

ObjString* takeString(int length, char chars[length]) {
  return allocateString(length, chars);
}
