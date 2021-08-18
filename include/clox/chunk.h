#ifndef CLOX_CHUNK_H_
#define CLOX_CHUNK_H_

#include "attributes.h"
#include "common.h"
#include "line.h"
#include "value.h"

#define OPCODES_ \
  X(CONSTANT) \
  X(CONSTANT_LONG) \
  X(NIL) \
  X(TRUE) \
  X(FALSE) \
  X(POP) \
  X(EQUAL) \
  X(GREATER) \
  X(LESS) \
  X(ADD) \
  X(SUBTRACT) \
  X(MULTIPLY) \
  X(DIVIDE) \
  X(NOT) \
  X(NEGATE) \
  X(PRINT) \
  X(RETURN)

typedef enum op_code_e
{
#define X(x) OP_##x,
  OPCODES_
#undef X
} OpCode;

extern const char* const g_OP_CODE_NAMES[];

typedef struct chunk_s {
  int count;
  int capacity;
  uint8_t* code;
  LineArray lines;
  ValueArray constants;
} Chunk;

void initChunk(Chunk* chunk) ATTR_NONNULL(1);
void writeChunk(Chunk* chunk, uint8_t byte, int line) ATTR_NONNULL(1);
void freeChunk(Chunk* chunk) ATTR_NONNULL(1);
int addConstant(Chunk* chunk, Value value) ATTR_NONNULL(1);
int writeConstant(Chunk* chunk, Value value, int line) ATTR_NONNULL(1);

#endif
