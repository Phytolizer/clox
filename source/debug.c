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
  printf("%-16s %4ud '", name, constant);
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
    case OP_DEFINE_GLOBAL:
    case OP_GET_GLOBAL:
      return constantInstruction(g_OP_CODE_NAMES[instruction], chunk, offset);
    case OP_CONSTANT_LONG:
    case OP_DEFINE_GLOBAL_LONG:
    case OP_GET_GLOBAL_LONG:
      return constantLongInstruction(
          g_OP_CODE_NAMES[instruction],
          chunk,
          offset);
    case OP_NIL:
    case OP_TRUE:
    case OP_FALSE:
    case OP_POP:
    case OP_EQUAL:
    case OP_GREATER:
    case OP_LESS:
    case OP_ADD:
    case OP_SUBTRACT:
    case OP_MULTIPLY:
    case OP_DIVIDE:
    case OP_NOT:
    case OP_NEGATE:
    case OP_RETURN:
    case OP_PRINT:
      return simpleInstruction(g_OP_CODE_NAMES[instruction], offset);
    default:
      printf("Unknown opcode %d\n", instruction);
      return offset + 1;
  }
}
