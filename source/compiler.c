//
// Created by kyle on 8/15/21.
//

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <clox/chunk.h>
#include <clox/common.h>
#include <clox/compiler.h>
#include <clox/scanner.h>
#ifdef DEBUG_PRINT_CODE
#  include <clox/debug.h>
#endif

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

typedef void ParseFn(void);

typedef struct parse_rule_s {
  ParseFn* prefix;
  ParseFn* infix;
  Precedence precedence;
} ParseRule;

static void grouping(void);
static void unary(void);
static void binary(void);
static void number(void);
static void literal(void);

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
    [TOKEN_BANG_EQUAL] = {NULL, NULL, PREC_NONE},
    [TOKEN_EQUAL] = {NULL, NULL, PREC_NONE},
    [TOKEN_EQUAL_EQUAL] = {NULL, NULL, PREC_NONE},
    [TOKEN_GREATER] = {NULL, NULL, PREC_NONE},
    [TOKEN_GREATER_EQUAL] = {NULL, NULL, PREC_NONE},
    [TOKEN_LESS] = {NULL, NULL, PREC_NONE},
    [TOKEN_LESS_EQUAL] = {NULL, NULL, PREC_NONE},
    [TOKEN_IDENTIFIER] = {NULL, NULL, PREC_NONE},
    [TOKEN_STRING] = {NULL, NULL, PREC_NONE},
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
Chunk* g_COMPILING_CHUNK;

static Chunk* currentChunk() {
  return g_COMPILING_CHUNK;
}

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

static void endCompiler() {
  emitReturn();
#ifdef DEBUG_PRINT_CODE
  if (!g_PARSER.hadError) {
    disassembleChunk(currentChunk(), "code");
  }
#endif
}

static void number() {
  double value = strtod(g_PARSER.previous.start, NULL);
  emitConstant(NUMBER_VAL(value));
}

static void parsePrecedence(Precedence precedence);

static void expression() {
  parsePrecedence(PREC_ASSIGNMENT);
}

static void grouping() {
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void unary() {
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

static void binary() {
  TokenType operatorType = g_PARSER.previous.type;
  const ParseRule* rule = getRule(operatorType);
  parsePrecedence((Precedence)(rule->precedence + 1));

  switch (operatorType) {
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

  prefixRule();

  while (precedence <= getRule(g_PARSER.current.type)->precedence) {
    advance();
    ParseFn* infixRule = getRule(g_PARSER.previous.type)->infix;
    infixRule();
  }
}

void literal(void) {
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

bool compile(const char source[static 1], Chunk* chunk) {
  initScanner(source);

  g_COMPILING_CHUNK = chunk;
  g_PARSER.hadError = false;
  g_PARSER.panicMode = false;

  advance();
  expression();
  consume(TOKEN_EOF, "Expect end of expression.");
  endCompiler();
  return !g_PARSER.hadError;
}
