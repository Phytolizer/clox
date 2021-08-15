#include <stdio.h>

#include <clox/vm.h>

static Vm g_VM;

void initVm() {
}

void freeVm() {
}

static InterpretResult run() {
#define READ_BYTE() (*g_VM.ip++)
#define READ_CONSTANT() (g_VM.chunk->constants.values[READ_BYTE()])
#define READ_LONG_CONSTANT() \
  ({ \
    uint32_t constant = READ_BYTE(); \
    constant += (uint32_t)READ_BYTE() * UINT8_COUNT; \
    constant += (uint32_t)READ_BYTE() * UINT8_COUNT * UINT8_COUNT; \
    g_VM.chunk->constants.values[constant]; \
  })
  for (;;) {
    uint8_t instruction;
    switch ((instruction = READ_BYTE())) {
      case OP_CONSTANT:
        {
          Value constant = READ_CONSTANT();
          printValue(constant);
          printf("\n");
          break;
        }
      case OP_CONSTANT_LONG:
        {
          Value constant = READ_LONG_CONSTANT();
          printValue(constant);
          printf("\n");
          break;
        }
      case OP_RETURN:
        {
          return INTERPRET_OK;
        }
    }
  }
#undef READ_LONG_CONSTANT
#undef READ_CONSTANT
#undef READ_BYTE
}

InterpretResult interpret(Chunk* chunk) {
  g_VM.chunk = chunk;
  g_VM.ip = chunk->code;
  return run();
}
