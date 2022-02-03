#pragma once

#include "common.h"

#define GROW_CAPACITY(Capacity) ((Capacity) < 8 ? 8 : (Capacity)*2)

#define GROW_ARRAY(Type, Pointer, OldCount, NewCount)   \
  (Type*)reallocate(Pointer, sizeof(Type) * (OldCount), \
                    sizeof(Type) * (NewCount))

#define FREE_ARRAY(Type, Pointer, OldCount) \
  reallocate(Pointer, sizeof(Type) * (OldCount), 0)

void* reallocate(void* pointer, Size oldSize, Size newSize);
