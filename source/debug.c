#include <stdio.h>

#include <clox/debug.h>
#include <clox/value.h>

static int simpleInstruction(const char* name, int offset) {
  printf("%s\n", name);
  return offset + 1;
}

static int constantInstruction(const char* name, Chunk* chunk, int offset) {
  uint8_t constant = chunk->code[offset + 1];
  printf("%-16s %4d '", name, constant);
  printValue(chunk->constants.values[constant]);
  fputs("'\n", stdout);
  return offset + 2;
}

static int constantLongInstruction(const char* name, Chunk* chunk, int offset) {
  uint32_t constant = chunk->code[offset + 1];
  constant += (uint32_t)(chunk->code[offset + 2]) * UINT8_COUNT;
  constant += (uint32_t)(chunk->code[offset + 3]) * UINT8_COUNT * UINT8_COUNT;
  printf("%-16s %4d '", name, constant);
  printValue(chunk->constants.values[constant]);
  fputs("'\n", stdout);
  return offset + 4;
}

void disassembleChunk(Chunk* chunk, const char* name) {
  printf("== %s ==\n", name);

  for (int offset = 0; offset < chunk->count;) {
    offset = disassembleInstruction(chunk, offset);
  }
}

int disassembleInstruction(Chunk* chunk, int offset) {
  printf("%04d ", offset);
  if (offset > 0
      && getLine(&chunk->lines, offset) == getLine(&chunk->lines, offset - 1)) {
    fputs("   | ", stdout);
  } else {
    printf("%4d ", getLine(&chunk->lines, offset));
  }
  uint8_t instruction = chunk->code[offset];
  switch (instruction) {
    case OP_CONSTANT:
      return constantInstruction("OP_CONSTANT", chunk, offset);
    case OP_CONSTANT_LONG:
      return constantLongInstruction("OP_CONSTANT_LONG", chunk, offset);
    case OP_RETURN:
      return simpleInstruction("OP_RETURN", offset);
    default:
      printf("Unknown opcode %d\n", instruction);
      return offset + 1;
  }
}
