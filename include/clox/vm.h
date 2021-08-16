#ifndef CLOX_VM_H_
#define CLOX_VM_H_

#include "chunk.h"

#define STACK_MAX 256

typedef struct vm_s {
  Chunk* chunk;
  uint8_t* ip;
  ValueArray stack;
  Value* stackTop;
} Vm;

typedef enum interpret_result_e
{
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR,
} InterpretResult;

void initVm();
void freeVm();
InterpretResult interpret(const char source[static 1]);
void push(Value value);
Value pop(void);

#endif
