#include <stdlib.h>

#include "chunk.h"
#include "memory.h"

void initChunk(Chunk* chunk) {
  chunk->count = 0;
  chunk->capacity = 0;
  chunk->code = NULL;
}

void freeChunk(Chunk* chunk) {
  FREE_ARRAY(U8, chunk->code, chunk->capacity);
  initChunk(chunk);
}

void writeChunk(Chunk* chunk, U8 byte) {
  if (chunk->capacity < chunk->count + 1) {
    I32 oldCapacity = chunk->capacity;
    chunk->capacity = GROW_CAPACITY(oldCapacity);
    chunk->code = GROW_ARRAY(U8, chunk->code, oldCapacity, chunk->capacity);
  }

  chunk->code[chunk->count] = byte;
  chunk->count += 1;
}
