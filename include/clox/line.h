#ifndef CLOX_LINE_H_
#define CLOX_LINE_H_

typedef struct line_s {
  int line;
  int length;
} Line;

typedef struct line_array_s {
  int count;
  int capacity;
  Line* lines;
} LineArray;

void initLineArray(LineArray* array);
void freeLineArray(LineArray* array);
void addLineArray(LineArray* array, int line);
int getLine(LineArray* array, int offset);

#endif
