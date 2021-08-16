#include <stdlib.h>

#include <clox/chunk.h>
#include <clox/line.h>
#include <clox/memory.h>
#include <clox/value.h>

void initChunk(Chunk* chunk) {
  chunk->count = 0;
  chunk->capacity = 0;
  chunk->code = NULL;
  initLineArray(&chunk->lines);
  initValueArray(&chunk->constants);
}

void writeChunk(Chunk* chunk, uint8_t byte, int line) {
  if (chunk->capacity < chunk->count + 1) {
    int oldCapacity = chunk->capacity;
    chunk->capacity = GROW_CAPACITY(oldCapacity);
    chunk->code
        = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
  }
  chunk->code[chunk->count] = byte;
  addLineArray(&chunk->lines, line);
  chunk->count++;
}

void freeChunk(Chunk* chunk) {
  FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
  freeLineArray(&chunk->lines);
  freeValueArray(&chunk->constants);
  initChunk(chunk);
}

int addConstant(Chunk* chunk, Value value) {
  writeValueArray(&chunk->constants, value);
  return chunk->constants.count - 1;
}

int writeConstant(Chunk* chunk, Value value, int line) {
  int constant = addConstant(chunk, value);
  if (constant > UINT8_MAX) {
    writeChunk(chunk, OP_CONSTANT_LONG, line);
    writeChunk(chunk, constant % UINT8_COUNT, line);
    constant /= UINT8_COUNT;
    writeChunk(chunk, constant % UINT8_COUNT, line);
    constant /= UINT8_COUNT;
    writeChunk(chunk, constant % UINT8_COUNT, line);
  } else {
    writeChunk(chunk, OP_CONSTANT, line);
    writeChunk(chunk, constant, line);
  }
  return constant;
}
