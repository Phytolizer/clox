//
// Created by kyle on 8/15/21.
//

#ifndef CLOX_OBJECT_H_
#define CLOX_OBJECT_H_

#include "common.h"
#include "value.h"

typedef enum obj_type_e
{ OBJ_STRING, } ObjType;

struct obj_s {
  ObjType type;
  struct obj_s* next;
};

struct obj_string_s {
  Obj obj;
  int length;
  char* chars;
};

ObjString* copyString(int length, const char chars[length]);
ObjString* takeString(int length, char chars[length]);
void printObject(Value value);

#define IS_OBJ_TYPE(value, objType) \
  ({ \
    __typeof(value) value_ = (value); \
    IS_OBJ(value_) && AS_OBJ(value_)->type == objType; \
  })

#define OBJ_TYPE(value) (AS_OBJ(value)->type)
#define IS_STRING(value) IS_OBJ_TYPE(value, OBJ_STRING)

#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)

#endif    // CLOX_OBJECT_H_
