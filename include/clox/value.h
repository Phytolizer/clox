#ifndef CLOX_VALUE_H_
#define CLOX_VALUE_H_

#include "attributes.h"
#include "common.h"

typedef struct obj_s Obj;
typedef struct obj_string_s ObjString;

typedef enum value_type_e
{
  VAL_BOOL,
  VAL_NIL,
  VAL_NUMBER,
  VAL_OBJ,
} ValueType;

typedef struct value_s {
  ValueType type;
  union value_u {
    bool boolean;
    double number;
    Obj* obj;
  } as;
} Value;

#define BOOL_VAL(value) ((Value){.type = VAL_BOOL, .as = {.boolean = (value)}})
#define NIL_VAL ((Value){.type = VAL_NIL, .as = {.number = 0}})
#define NUMBER_VAL(value) \
  ((Value){.type = VAL_NUMBER, .as = {.number = (value)}})
#define OBJ_VAL(object) \
  ((Value){.type = VAL_OBJ, .as = {.obj = (Obj*)(object)}})

#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NIL(value) ((value).type == VAL_NIL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define IS_OBJ(value) ((value).type == VAL_OBJ)

#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)
#define AS_OBJ(value) ((value).as.obj)

typedef struct value_array_s {
  int capacity;
  int count;
  Value* values;
} ValueArray;

void initValueArray(ValueArray* array) ATTR_NONNULL(1);
void writeValueArray(ValueArray* array, Value value) ATTR_NONNULL(1);
void freeValueArray(ValueArray* array) ATTR_NONNULL(1);
void printValue(Value value);

#endif
