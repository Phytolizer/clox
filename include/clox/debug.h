#ifndef CLOX_DEBUG_H_
#define CLOX_DEBUG_H_

#include "attributes.h"
#include "chunk.h"

void disassembleChunk(Chunk* chunk, const char* name) ATTR_NONNULL(1);
int disassembleInstruction(Chunk* chunk, int offset) ATTR_NONNULL(1);

#endif
