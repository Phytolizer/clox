#include <stdlib.h>

#include <clox/line.h>
#include <clox/memory.h>

void initLineArray(LineArray* array)
{
  array->lines = NULL;
  array->capacity = 0;
  array->count = 0;
}

void freeLineArray(LineArray* array)
{
  FREE_ARRAY(Line, array->lines, array->capacity);
  initLineArray(array);
}

void addLineArray(LineArray* array, int line)
{
  if (array->count > 0 && array->lines[array->count - 1].line == line)
  {
    array->lines[array->count - 1].length++;
  }
  else
  {
    if (array->capacity < array->count + 1)
    {
      int oldCapacity = array->capacity;
      array->capacity = GROW_CAPACITY(oldCapacity);
      array->lines
          = GROW_ARRAY(Line, array->lines, oldCapacity, array->capacity);
    }
    array->lines[array->count].line = line;
    array->lines[array->count].length = 1;
    array->count++;
  }
}

int getLine(LineArray* array, int offset)
{
  int offseti = 0;
  for (int i = 0; i < array->count; i++)
  {
    offseti += array->lines[i].length;
    if (offseti > offset)
    {
      return array->lines[i].line;
    }
  }
  return 0;
}
