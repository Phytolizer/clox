#ifndef CLOX_VALUE_H_
#define CLOX_VALUE_H_

#include "common.h"

typedef enum value_type_e
{
  VAL_BOOL,
  VAL_NIL,
  VAL_NUMBER,
} ValueType;

typedef struct value_s {
  ValueType type;
  union value_u {
    bool boolean;
    double number;
  } as;
} Value;

#define BOOL_VAL(value) ((Value){.type = VAL_BOOL, .as = {.boolean = (value)}})
#define NIL_VAL ((Value){.type = VAL_NIL, .as = {.number = 0}})
#define NUMBER_VAL(value) \
  ((Value){.type = VAL_NUMBER, .as = {.number = (value)}})

#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NIL(value) ((value).type == VAL_NIL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)

#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)

typedef struct value_array_s {
  int capacity;
  int count;
  Value* values;
} ValueArray;

void initValueArray(ValueArray* array);
void writeValueArray(ValueArray* array, Value value);
void freeValueArray(ValueArray* array);
void printValue(Value value);

#endif
