#pragma once

#include "common.h"

#define OPCODES_X X(RETURN)

typedef enum {
#define X(name) OP_##name,
  OPCODES_X
#undef X
} OpCode;

typedef struct {
  I32 count;
  I32 capacity;
  U8* code;
} Chunk;

void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, U8 byte);
