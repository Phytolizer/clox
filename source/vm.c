#pragma region "includes"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <clox/compiler.h>
#include <clox/memory.h>
#include <clox/vm.h>

#ifdef DEBUG_TRACE_EXECUTION
#  include <clox/debug.h>
#endif

#pragma endregion

// not static due to usage in other files
Vm g_VM;

#pragma region "error handling"

static void resetStack() {
  g_VM.stackTop = g_VM.stack.values;
}

static void runtimeError(const char format[static 1], ...)
    __attribute__((format(printf, 1, 2)));

static void runtimeError(const char format[static 1], ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  size_t instruction = g_VM.ip - g_VM.chunk->code - 1;
  int line = getLine(&g_VM.chunk->lines, (int)instruction);
  fprintf(stderr, "[line %d] in script\n", line);
  resetStack();
}

#pragma endregion

#pragma region "init/deinit"

void initVm() {
  initValueArray(&g_VM.stack);
  g_VM.objects = NULL;
  resetStack();
  initTable(&g_VM.strings);
  initTable(&g_VM.globals);
}

void freeVm() {
  freeObjects();
  freeTable(&g_VM.strings);
  freeTable(&g_VM.globals);
}

#pragma endregion

#pragma region "utility functions"

static Value peek(int distance) {
  return g_VM.stackTop[-1 - distance];
}

static bool isFalsey(Value value) {
  return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static bool valuesEqual(Value a, Value b) {
  switch (a.type) {
    case VAL_BOOL:
      return IS_BOOL(b) && AS_BOOL(a) == AS_BOOL(b);
    case VAL_NIL:
      return IS_NIL(b);
    case VAL_NUMBER:
      return IS_NUMBER(b) && AS_NUMBER(a) == AS_NUMBER(b);
    case VAL_OBJ:
      return AS_OBJ(a) == AS_OBJ(b);
  }
}

static void concatenate() {
  ObjString* b = AS_STRING(pop());
  ObjString* a = AS_STRING(pop());

  int length = a->length + b->length;
  char* chars = ALLOCATE(char, length + 1);
  memcpy(chars, a->chars, a->length);
  memcpy(chars + a->length, b->chars, b->length);
  chars[length] = 0;
  ObjString* result = takeString(length, chars);
  push(OBJ_VAL(result));
}

#pragma endregion

#pragma region "the hot function, run()"

static InterpretResult run() {
#define READ_BYTE() (*g_VM.ip++)
#define READ_THREE_BYTES() \
  (READ_BYTE() + READ_BYTE() * UINT8_COUNT \
   + READ_BYTE() * UINT8_COUNT * UINT8_COUNT)
#define READ_CONSTANT() (g_VM.chunk->constants.values[READ_BYTE()])
#define READ_LONG_CONSTANT() (g_VM.chunk->constants.values[READ_THREE_BYTES()])
#define READ_STRING() AS_STRING(READ_CONSTANT())
#define READ_STRING_LONG() AS_STRING(READ_LONG_CONSTANT())
#define BINARY_OP(valueType, op) \
  do { \
    if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
      runtimeError("Operands must be numbers."); \
      return INTERPRET_RUNTIME_ERROR; \
    } \
    double b = AS_NUMBER(pop()); \
    double a = AS_NUMBER(pop()); \
    push(valueType(a op b)); \
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
      case OP_NIL:
        push(NIL_VAL);
        break;
      case OP_TRUE:
        push(BOOL_VAL(true));
        break;
      case OP_FALSE:
        push(BOOL_VAL(false));
        break;
      case OP_POP:
        pop();
        break;
      case OP_DEFINE_GLOBAL:
        {
          ObjString* name = READ_STRING();
          tableSet(&g_VM.globals, name, peek(0));
          pop();
          break;
        }
      case OP_DEFINE_GLOBAL_LONG:
        {
          ObjString* name = READ_STRING_LONG();
          tableSet(&g_VM.globals, name, peek(0));
          pop();
          break;
        }
      case OP_GET_GLOBAL:
        {
          ObjString* name = READ_STRING();
          Value value;
          if (!tableGet(&g_VM.globals, name, &value)) {
            runtimeError("Undefined variable '%s'", name->chars);
            return INTERPRET_RUNTIME_ERROR;
          }
          push(value);
          break;
        }
      case OP_GET_GLOBAL_LONG:
        {
          ObjString* name = READ_STRING_LONG();
          Value value;
          if (!tableGet(&g_VM.globals, name, &value)) {
            runtimeError("Undefined variable '%s'", name->chars);
            return INTERPRET_RUNTIME_ERROR;
          }
          push(value);
          break;
        }
      case OP_SET_GLOBAL:
      case OP_SET_GLOBAL_LONG:
        {
          ObjString* name = (instruction == OP_SET_GLOBAL) ? READ_STRING()
                                                           : READ_STRING_LONG();
          if (tableSet(&g_VM.globals, name, peek(0))) {
            tableDelete(&g_VM.globals, name);
            runtimeError("Undefined variable '%s'", name->chars);
            return INTERPRET_RUNTIME_ERROR;
          }
          break;
        }
      case OP_GET_LOCAL:
      case OP_GET_LOCAL_LONG:
        {
          uint8_t slot = (instruction == OP_GET_LOCAL) ? READ_BYTE()
                                                       : READ_THREE_BYTES();
          push(g_VM.stack.values[slot]);
          break;
        }
      case OP_SET_LOCAL:
      case OP_SET_LOCAL_LONG:
        {
          uint8_t slot = (instruction == OP_SET_LOCAL) ? READ_BYTE()
                                                       : READ_THREE_BYTES();
          g_VM.stack.values[slot] = pop();
          break;
        }
      case OP_EQUAL:
        {
          Value b = pop();
          Value a = pop();
          push(BOOL_VAL(valuesEqual(a, b)));
          break;
        }
      case OP_GREATER:
        BINARY_OP(BOOL_VAL, >);
        break;
      case OP_LESS:
        BINARY_OP(BOOL_VAL, <);
        break;
      case OP_ADD:
        if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
          concatenate();
        } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
          double b = AS_NUMBER(pop());
          double a = AS_NUMBER(pop());
          push(NUMBER_VAL(a + b));
        } else {
          runtimeError("Operands must be two numbers or two strings.");
          return INTERPRET_RUNTIME_ERROR;
        }
        break;
      case OP_SUBTRACT:
        BINARY_OP(NUMBER_VAL, -);
        break;
      case OP_MULTIPLY:
        BINARY_OP(NUMBER_VAL, *);
        break;
      case OP_DIVIDE:
        BINARY_OP(NUMBER_VAL, /);
        break;
      case OP_NOT:
        push(BOOL_VAL(isFalsey(pop())));
        break;
      case OP_NEGATE:
        {
          if (!IS_NUMBER(peek(0))) {
            runtimeError("Operand must be a number.");
            return INTERPRET_RUNTIME_ERROR;
          }
          push(NUMBER_VAL(-AS_NUMBER(pop())));
          break;
        }
      case OP_PRINT:
        {
          printValue(pop());
          fputs("\n", stdout);
          break;
        }
      case OP_RETURN:
        {
          return INTERPRET_OK;
        }
    }
  }
#undef BINARY_OP
#undef READ_STRING_LONG
#undef READ_STRING
#undef READ_LONG_CONSTANT
#undef READ_CONSTANT
#undef READ_BYTE
}

#pragma endregion

#define CHUNK_CLEANUP ATTR_CLEANUP(freeChunk)

InterpretResult interpret(const char source[static 1]) {
  Chunk CHUNK_CLEANUP chunk;
  initChunk(&chunk);

  if (!compile(source, &chunk)) {
    return INTERPRET_COMPILE_ERROR;
  }

  g_VM.chunk = &chunk;
  g_VM.ip = g_VM.chunk->code;

  return run();
}

#pragma region "stack manipulation"

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

#pragma endregion
