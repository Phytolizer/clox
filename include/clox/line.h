#ifndef CLOX_LINE_H_
#define CLOX_LINE_H_

#include "attributes.h"

typedef struct line_s {
  int line;
  int length;
} Line;

typedef struct line_array_s {
  int count;
  int capacity;
  Line* lines;
} LineArray;

void initLineArray(LineArray* array) ATTR_NONNULL(1);
void freeLineArray(LineArray* array) ATTR_NONNULL(1);
void addLineArray(LineArray* array, int line) ATTR_NONNULL(1);
int getLine(LineArray* array, int offset) ATTR_NONNULL(1);

#endif
