#include <stdio.h>

#include <clox/vm.h>

#ifdef DEBUG_TRACE_EXECUTION
#  include <clox/debug.h>
#endif

static Vm g_VM;

static void resetStack() {
  g_VM.stackTop = g_VM.stack.values;
}

void initVm() {
  initValueArray(&g_VM.stack);
  resetStack();
}

void freeVm() {
}

static InterpretResult run() {
#define READ_BYTE() (*g_VM.ip++)
#define READ_CONSTANT() (g_VM.chunk->constants.values[READ_BYTE()])
#define READ_LONG_CONSTANT() \
  ({ \
    uint32_t constant_ = READ_BYTE(); \
    constant_ += (uint32_t)READ_BYTE() * UINT8_COUNT; \
    constant_ += (uint32_t)READ_BYTE() * UINT8_COUNT * UINT8_COUNT; \
    g_VM.chunk->constants.values[constant_]; \
  })
#define BINARY_OP(op) \
  do { \
    double b = pop(); \
    double a = pop(); \
    push(a op b); \
  } while (false)
  for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
    for (size_t i = 0; i < 10; i++) {
      fputc(' ', stdout);
    }
    for (Value* slot = g_VM.stack.values; slot < g_VM.stackTop; slot++) {
      fputs("[ ", stdout);
      printValue(*slot);
      fputs(" ]", stdout);
    }
    fputs("\n", stdout);
    disassembleInstruction(g_VM.chunk, (int)(g_VM.ip - g_VM.chunk->code));
#endif
    uint8_t instruction = READ_BYTE();
    switch (instruction) {
      case OP_CONSTANT:
        {
          Value constant = READ_CONSTANT();
          push(constant);
          break;
        }
      case OP_CONSTANT_LONG:
        {
          Value constant = READ_LONG_CONSTANT();
          push(constant);
          break;
        }
      case OP_ADD:
        BINARY_OP(+);
        break;
      case OP_SUBTRACT:
        BINARY_OP(-);
        break;
      case OP_MULTIPLY:
        BINARY_OP(*);
        break;
      case OP_DIVIDE:
        BINARY_OP(/);
        break;
      case OP_NEGATE:
        {
          push(-pop());
          break;
        }
      case OP_RETURN:
        {
          printValue(pop());
          fputs("\n", stdout);
          return INTERPRET_OK;
        }
    }
  }
#undef BINARY_OP
#undef READ_LONG_CONSTANT
#undef READ_CONSTANT
#undef READ_BYTE
}

InterpretResult interpret(Chunk* chunk) {
  g_VM.chunk = chunk;
  g_VM.ip = chunk->code;
  return run();
}

void push(Value value) {
  if (g_VM.stackTop - g_VM.stack.values == g_VM.stack.count) {
    writeValueArray(&g_VM.stack, value);
    g_VM.stackTop = g_VM.stack.values + g_VM.stack.count;
  } else {
    *g_VM.stackTop = value;
    g_VM.stackTop++;
  }
}

Value pop(void) {
  g_VM.stackTop--;
  return *g_VM.stackTop;
}
