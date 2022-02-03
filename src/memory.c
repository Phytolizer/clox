#include <stdlib.h>

#include "memory.h"

void* reallocate(void* pointer, Size oldSize, Size newSize) {
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
