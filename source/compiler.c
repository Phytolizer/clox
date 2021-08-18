//
// Created by kyle on 8/15/21.
//

#pragma region "includes"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <clox/chunk.h>
#include <clox/common.h>
#include <clox/compiler.h>
#include <clox/scanner.h>
#ifdef DEBUG_PRINT_CODE
#  include <clox/debug.h>
#endif

#pragma endregion

#pragma region "types"

typedef struct parser_s {
  Token current;
  Token previous;
  bool hadError;
  bool panicMode;
} Parser;

#define PRECEDENCES_ \
  X(NONE) \
  X(ASSIGNMENT) \
  X(OR) \
  X(AND) \
  X(EQUALITY) \
  X(COMPARISON) \
  X(TERM) \
  X(FACTOR) \
  X(UNARY) \
  X(CALL) \
  X(PRIMARY)

typedef enum precedence_e
{
#define X(x) PREC_##x,
  PRECEDENCES_
#undef X
} Precedence;

typedef void ParseFn(bool canAssign);

typedef struct parse_rule_s {
  ParseFn* prefix;
  ParseFn* infix;
  Precedence precedence;
} ParseRule;

typedef struct local_s {
  Token name;
  int depth;
} Local;

typedef struct compiler_s {
  Local locals[UINT8_COUNT];
  int localCount;
  int scopeDepth;
} Compiler;

#pragma endregion

#pragma region "pre-declarations"

static void grouping(bool canAssign);
static void unary(bool canAssign);
static void binary(bool canAssign);
static void number(bool canAssign);
static void literal(bool canAssign);
static void string(bool canAssign);
static void declaration(void);
static void statement(void);
static void variable(bool canAssign);

#pragma endregion

#pragma region "constants and globals"

static const ParseRule g_RULES[] = {
    [TOKEN_LEFT_PAREN] = {grouping, NULL, PREC_NONE},
    [TOKEN_RIGHT_PAREN] = {NULL, NULL, PREC_NONE},
    [TOKEN_LEFT_BRACE] = {NULL, NULL, PREC_NONE},
    [TOKEN_RIGHT_BRACE] = {NULL, NULL, PREC_NONE},
    [TOKEN_COMMA] = {NULL, NULL, PREC_NONE},
    [TOKEN_DOT] = {NULL, NULL, PREC_NONE},
    [TOKEN_MINUS] = {unary, binary, PREC_TERM},
    [TOKEN_PLUS] = {NULL, binary, PREC_TERM},
    [TOKEN_SEMICOLON] = {NULL, NULL, PREC_NONE},
    [TOKEN_SLASH] = {NULL, binary, PREC_FACTOR},
    [TOKEN_STAR] = {NULL, binary, PREC_FACTOR},
    [TOKEN_BANG] = {unary, NULL, PREC_NONE},
    [TOKEN_BANG_EQUAL] = {NULL, binary, PREC_EQUALITY},
    [TOKEN_EQUAL] = {NULL, NULL, PREC_NONE},
    [TOKEN_EQUAL_EQUAL] = {NULL, binary, PREC_EQUALITY},
    [TOKEN_GREATER] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_GREATER_EQUAL] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_LESS] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_LESS_EQUAL] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_IDENTIFIER] = {variable, NULL, PREC_NONE},
    [TOKEN_STRING] = {string, NULL, PREC_NONE},
    [TOKEN_NUMBER] = {number, NULL, PREC_NONE},
    [TOKEN_AND] = {NULL, NULL, PREC_NONE},
    [TOKEN_CLASS] = {NULL, NULL, PREC_NONE},
    [TOKEN_ELSE] = {NULL, NULL, PREC_NONE},
    [TOKEN_FALSE] = {literal, NULL, PREC_NONE},
    [TOKEN_FOR] = {NULL, NULL, PREC_NONE},
    [TOKEN_FUN] = {NULL, NULL, PREC_NONE},
    [TOKEN_IF] = {NULL, NULL, PREC_NONE},
    [TOKEN_NIL] = {literal, NULL, PREC_NONE},
    [TOKEN_OR] = {NULL, NULL, PREC_NONE},
    [TOKEN_PRINT] = {NULL, NULL, PREC_NONE},
    [TOKEN_RETURN] = {NULL, NULL, PREC_NONE},
    [TOKEN_SUPER] = {NULL, NULL, PREC_NONE},
    [TOKEN_THIS] = {NULL, NULL, PREC_NONE},
    [TOKEN_TRUE] = {literal, NULL, PREC_NONE},
    [TOKEN_VAR] = {NULL, NULL, PREC_NONE},
    [TOKEN_WHILE] = {NULL, NULL, PREC_NONE},
    [TOKEN_ERROR] = {NULL, NULL, PREC_NONE},
    [TOKEN_EOF] = {NULL, NULL, PREC_NONE},
};

static const ParseRule* getRule(TokenType type) {
  return &g_RULES[type];
}

const char* const g_PRECEDENCE_NAMES[] = {
#define X(x) #x,
    PRECEDENCES_
#undef X
};

Parser g_PARSER;
Compiler* g_CURRENT = NULL;
Chunk* g_COMPILING_CHUNK;

#pragma endregion

static Chunk* currentChunk() {
  return g_COMPILING_CHUNK;
}

#pragma region "error handling"

static void errorAt(Token* token, const char message[static 1]) {
  if (g_PARSER.panicMode) {
    return;
  }
  g_PARSER.panicMode = true;
  fprintf(stderr, "[line %d] Error", token->line);

  if (token->type == TOKEN_EOF) {
    fputs(" at end", stderr);
  } else if (token->type != TOKEN_ERROR) {
    fprintf(stderr, " at '%.*s'", token->length, token->start);
  }

  fprintf(stderr, ": %s\n", message);
  g_PARSER.hadError = true;
}

static void error(const char message[static 1]) {
  errorAt(&g_PARSER.previous, message);
}

static void errorAtCurrent(const char message[static 1]) {
  errorAt(&g_PARSER.current, message);
}

#pragma endregion

#pragma region "parser helpers"

static void advance() {
  g_PARSER.previous = g_PARSER.current;

  for (;;) {
    g_PARSER.current = scanToken();
    if (g_PARSER.current.type != TOKEN_ERROR) {
      break;
    }

    errorAtCurrent(g_PARSER.current.start);
  }
}

static void consume(TokenType type, const char message[static 1]) {
  if (g_PARSER.current.type == type) {
    advance();
    return;
  }

  errorAtCurrent(message);
}

static bool check(TokenType type) {
  return g_PARSER.current.type == type;
}

static bool match(TokenType type) {
  if (!check(type)) {
    return false;
  }

  advance();
  return true;
}

#pragma endregion

#pragma region "compiler plumbing"

static void emitByte(uint8_t byte) {
  writeChunk(currentChunk(), byte, g_PARSER.previous.line);
}

static void emitBytes(uint8_t b1, uint8_t b2) {
  emitByte(b1);
  emitByte(b2);
}

static void emitReturn() {
  emitByte(OP_RETURN);
}

static uint32_t makeConstant(Value value) {
  return addConstant(currentChunk(), value);
}

static void emitConstant(Value value) {
  uint32_t constant = makeConstant(value);
  if (constant > UINT8_MAX) {
    emitByte(OP_CONSTANT_LONG);
    emitByte(constant % UINT8_COUNT);
    constant /= UINT8_COUNT;
    emitByte(constant % UINT8_COUNT);
    constant /= UINT8_COUNT;
    emitByte(constant % UINT8_COUNT);
  } else {
    emitBytes(OP_CONSTANT, constant);
  }
}

static void initCompiler(Compiler* compiler) {
  compiler->localCount = 0;
  compiler->scopeDepth = 0;
  g_CURRENT = compiler;
}

static void endCompiler() {
  emitReturn();
#ifdef DEBUG_PRINT_CODE
  if (!g_PARSER.hadError) {
    disassembleChunk(currentChunk(), "code");
  }
#endif
}

static void beginScope() {
  g_CURRENT->scopeDepth++;
}

static void endScope() {
  g_CURRENT->scopeDepth--;

  while (g_CURRENT->localCount > 0
         && g_CURRENT->locals[g_CURRENT->localCount - 1].depth
             > g_CURRENT->scopeDepth) {
    emitByte(OP_POP);
    g_CURRENT->localCount--;
  }
}

#pragma endregion

#pragma region "parsing functions"

static void number(bool canAssign) {
  double value = strtod(g_PARSER.previous.start, NULL);
  emitConstant(NUMBER_VAL(value));
}

static void parsePrecedence(Precedence precedence);

static uint32_t identifierConstant(Token* name) {
  return makeConstant(OBJ_VAL(copyString(name->length, name->start)));
}

static bool identifiersEqual(Token* a, Token* b) {
  if (a->length != b->length) {
    return false;
  }
  return memcmp(a->start, b->start, a->length) == 0;
}

static int resolveLocal(Compiler* compiler, Token* name) {
  for (int i = compiler->localCount - 1; i >= 0; --i) {
    Local* local = &compiler->locals[i];
    if (identifiersEqual(name, &local->name)) {
      if (local->depth == -1) {
        error("Can't read local variable in its own initializer.");
      }
      return i;
    }
  }

  return -1;
}

static void addLocal(Token name) {
  if (g_CURRENT->localCount == UINT8_COUNT) {
    error("Too many local variables in function.");
    return;
  }

  Local* local = &g_CURRENT->locals[g_CURRENT->localCount++];
  local->name = name;
  local->depth = -1;
}

static void declareVariable() {
  if (g_CURRENT->scopeDepth == 0) {
    return;
  }

  Token* name = &g_PARSER.previous;

  for (int i = g_CURRENT->localCount - 1; i >= 0; --i) {
    Local* local = &g_CURRENT->locals[i];
    if (local->depth != -1 && local->depth < g_CURRENT->scopeDepth) {
      break;
    }

    if (identifiersEqual(name, &local->name)) {
      error("Already a variable with this name in this scope.");
    }
  }

  addLocal(*name);
}

static uint32_t parseVariable(const char errorMessage[static 1]) {
  consume(TOKEN_IDENTIFIER, errorMessage);
  return identifierConstant(&g_PARSER.previous);
}

static void markInitialized() {
  g_CURRENT->locals[g_CURRENT->localCount - 1].depth = g_CURRENT->scopeDepth;
}

static void defineVariable(uint32_t global) {
  if (g_CURRENT->scopeDepth > 0) {
    markInitialized();
    return;
  }

  if (global <= UINT8_MAX) {
    emitByte(OP_DEFINE_GLOBAL);
    emitByte(global);
  } else {
    emitByte(OP_DEFINE_GLOBAL_LONG);
    emitByte(global % UINT8_COUNT);
    global /= UINT8_COUNT;
    emitByte(global % UINT8_COUNT);
    global /= UINT8_COUNT;
    emitByte(global % UINT8_COUNT);
  }
}

static void expression() {
  parsePrecedence(PREC_ASSIGNMENT);
}

static void block() {
  while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
    declaration();
  }

  consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

static void varDeclaration() {
  uint32_t global = parseVariable("Expect variable name.");

  if (match(TOKEN_EQUAL)) {
    expression();
  } else {
    emitByte(OP_NIL);
  }

  consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");

  defineVariable(global);
}

static void expressionStatement() {
  expression();
  consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
  emitByte(OP_POP);
}

static void printStatement() {
  expression();
  consume(TOKEN_SEMICOLON, "Expect ';' after value.");
  emitByte(OP_PRINT);
}

static void synchronize() {
  g_PARSER.panicMode = false;
  while (g_PARSER.current.type != TOKEN_EOF) {
    if (g_PARSER.previous.type == TOKEN_SEMICOLON) {
      return;
    }
    switch (g_PARSER.current.type) {
      case TOKEN_CLASS:
      case TOKEN_FUN:
      case TOKEN_VAR:
      case TOKEN_FOR:
      case TOKEN_IF:
      case TOKEN_WHILE:
      case TOKEN_PRINT:
      case TOKEN_RETURN:
        return;
      default:
        break;
    }

    advance();
  }
}

static void declaration() {
  if (match(TOKEN_VAR)) {
    varDeclaration();
  } else {
    statement();
  }

  if (g_PARSER.panicMode) {
    synchronize();
  }
}

static void statement() {
  if (match(TOKEN_PRINT)) {
    printStatement();
  } else if (match(TOKEN_LEFT_BRACE)) {
    beginScope();
    block();
    endScope();
  } else {
    expressionStatement();
  }
}

static void namedVariable(Token name, bool canAssign) {
  OpCode get_op = OP_GET_GLOBAL;
  OpCode get_op_long = OP_GET_GLOBAL_LONG;
  OpCode set_op = OP_SET_GLOBAL;
  OpCode set_op_long = OP_SET_GLOBAL_LONG;

  int arg = resolveLocal(g_CURRENT, &name);
  if (arg != -1) {
    get_op = OP_GET_LOCAL;
    get_op_long = OP_GET_LOCAL_LONG;
    set_op = OP_SET_LOCAL;
    set_op_long = OP_SET_LOCAL_LONG;
  } else {
    arg = identifierConstant(&name);
  }

  OpCode op = get_op;
  OpCode op_long = get_op_long;

  if (canAssign && match(TOKEN_EQUAL)) {
    expression();
    op = set_op;
    op_long = set_op_long;
  }
  if (arg <= UINT8_MAX) {
    emitBytes(op, arg);
  } else {
    emitBytes(op_long, arg % UINT8_COUNT);
    arg /= UINT8_COUNT;
    emitByte(arg % UINT8_COUNT);
    arg /= UINT8_COUNT;
    emitByte(arg % UINT8_COUNT);
  }
}

static void variable(bool canAssign) {
  namedVariable(g_PARSER.previous, canAssign);
}

static void grouping(bool canAssign) {
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void unary(bool canAssign) {
  TokenType operatorType = g_PARSER.previous.type;

  parsePrecedence(PREC_UNARY);

  switch (operatorType) {
    case TOKEN_BANG:
      emitByte(OP_NOT);
      break;
    case TOKEN_MINUS:
      emitByte(OP_NEGATE);
      break;
    default:
      assert(false);
  }
}

static void binary(bool canAssign) {
  TokenType operatorType = g_PARSER.previous.type;
  const ParseRule* rule = getRule(operatorType);
  parsePrecedence((Precedence)(rule->precedence + 1));

  switch (operatorType) {
    case TOKEN_BANG_EQUAL:
      emitBytes(OP_EQUAL, OP_NOT);
      break;
    case TOKEN_EQUAL_EQUAL:
      emitByte(OP_EQUAL);
      break;
    case TOKEN_GREATER:
      emitByte(OP_GREATER);
      break;
    case TOKEN_GREATER_EQUAL:
      emitBytes(OP_LESS, OP_NOT);
      break;
    case TOKEN_LESS:
      emitByte(OP_LESS);
      break;
    case TOKEN_LESS_EQUAL:
      emitBytes(OP_GREATER, OP_NOT);
      break;
    case TOKEN_PLUS:
      emitByte(OP_ADD);
      break;
    case TOKEN_MINUS:
      emitByte(OP_SUBTRACT);
      break;
    case TOKEN_STAR:
      emitByte(OP_MULTIPLY);
      break;
    case TOKEN_SLASH:
      emitByte(OP_DIVIDE);
      break;
    default:
      assert(false);
  }
}

static void parsePrecedence(Precedence precedence) {
  advance();
  ParseFn* prefixRule = getRule(g_PARSER.previous.type)->prefix;
  if (!prefixRule) {
    error("Expect expression.");
    return;
  }

  bool canAssign = precedence <= PREC_ASSIGNMENT;
  prefixRule(canAssign);

  while (precedence <= getRule(g_PARSER.current.type)->precedence) {
    advance();
    ParseFn* infixRule = getRule(g_PARSER.previous.type)->infix;
    infixRule(canAssign);
  }

  if (canAssign && match(TOKEN_EQUAL)) {
    error("Invalid assignment target.");
  }
}

void literal(bool canAssign) {
  switch (g_PARSER.previous.type) {
    case TOKEN_FALSE:
      emitByte(OP_FALSE);
      break;
    case TOKEN_NIL:
      emitByte(OP_NIL);
      break;
    case TOKEN_TRUE:
      emitByte(OP_TRUE);
      break;
    default:
      assert(false);
  }
}

void string(bool canAssign) {
  emitConstant(OBJ_VAL(
      copyString(g_PARSER.previous.length - 2, g_PARSER.previous.start + 1)));
}

#pragma endregion

bool compile(const char source[static 1], Chunk* chunk) {
  initScanner(source);

  Compiler compiler;
  initCompiler(&compiler);
  g_COMPILING_CHUNK = chunk;
  g_PARSER.hadError = false;
  g_PARSER.panicMode = false;

  advance();

  while (!match(TOKEN_EOF)) {
    declaration();
  }

  endCompiler();
  return !g_PARSER.hadError;
}
