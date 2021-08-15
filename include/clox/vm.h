#ifndef CLOX_VM_H_
#define CLOX_VM_H_

#include "chunk.h"

typedef struct vm_s {
  Chunk* chunk;
  uint8_t* ip;
} Vm;

typedef enum interpret_result_e
{
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR,
} InterpretResult;

void initVm();
void freeVm();
InterpretResult interpret(Chunk* chunk);

#endif
