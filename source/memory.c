#include <stdlib.h>

#include <clox/memory.h>
#include <clox/object.h>
#include <clox/value.h>
#include <clox/vm.h>

extern Vm g_VM;

static void freeObject(Obj* object);
void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
  if (newSize == 0) {
    free(pointer);
    return NULL;
  }

  void* result = realloc(pointer, newSize);
  if (result == NULL) {
    exit(1);
  }
  return result;
}
void freeObjects(void) {
  Obj* object = g_VM.objects;
  while (object) {
    Obj* next = object->next;
    freeObject(object);
    object = next;
  }
}
static void freeObject(Obj* object) {
  switch (object->type) {
    case OBJ_STRING:
      {
        ObjString* string = (ObjString*)object;
        FREE_ARRAY(char, string->chars, string->length + 1);
        FREE(ObjString, object);
        break;
      }
  }
}
