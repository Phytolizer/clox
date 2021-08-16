//
// Created by kyle on 8/15/21.
//

#ifndef SCANNER_H_
#define SCANNER_H_

#define TOKENS_ \
  X(LEFT_PAREN) \
  X(RIGHT_PAREN) \
  X(LEFT_BRACE) \
  X(RIGHT_BRACE) \
  X(COMMA) \
  X(DOT) \
  X(MINUS) \
  X(PLUS) \
  X(SEMICOLON) \
  X(SLASH) \
  X(STAR) \
\
  X(BANG) \
  X(BANG_EQUAL) \
  X(EQUAL) \
  X(EQUAL_EQUAL) \
  X(GREATER) \
  X(GREATER_EQUAL) \
  X(LESS) \
  X(LESS_EQUAL) \
\
  X(IDENTIFIER) \
  X(STRING) \
  X(NUMBER) \
\
  X(AND) \
  X(CLASS) \
  X(ELSE) \
  X(FALSE) \
  X(FOR) \
  X(FUN) \
  X(IF) \
  X(NIL) \
  X(OR) \
  X(PRINT) \
  X(RETURN) \
  X(SUPER) \
  X(THIS) \
  X(TRUE) \
  X(VAR) \
  X(WHILE) \
\
  X(ERROR) \
  X(EOF)

typedef enum token_type_e
{
#define X(x) TOKEN_##x,
  TOKENS_
#undef X
} TokenType;

typedef struct token_s {
  TokenType type;
  const char* start;
  int length;
  int line;
} Token;

extern const char* TOKEN_NAMES[];

void initScanner(const char source[static 1]);
Token scanToken(void);

#endif    // SCANNER_H_
