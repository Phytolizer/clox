//
// Created by kyle on 8/15/21.
//

#include <stdio.h>
#include <string.h>

#include <clox/common.h>
#include <clox/scanner.h>

const char* TOKEN_NAMES[] = {
#define STRINGIZE(x) #x
#define X(x) STRINGIZE(TOKEN_##x),
    TOKENS_
#undef X
#undef STRINGIZE
};

typedef struct scanner_s {
  const char* start;
  const char* current;
  int line;
} Scanner;

Scanner g_SCANNER;

static bool isAtEnd() {
  return *g_SCANNER.current == '\0';
}

static Token makeToken(TokenType type) {
  return (Token){
      .type = type,
      .start = g_SCANNER.start,
      .length = (int)(g_SCANNER.current - g_SCANNER.start),
      .line = g_SCANNER.line,
  };
}

static Token errorToken(const char message[static 1]) {
  return (Token){
      .type = TOKEN_ERROR,
      .start = message,
      .length = (int)strlen(message),
      .line = g_SCANNER.line,
  };
}

static char advance() {
  g_SCANNER.current++;
  return g_SCANNER.current[-1];
}

static bool match(char expected) {
  if (isAtEnd()) {
    return false;
  }
  if (*g_SCANNER.current != expected) {
    return false;
  }
  g_SCANNER.current++;
  return true;
}

static char peek() {
  return *g_SCANNER.current;
}

static char peekNext() {
  if (isAtEnd()) {
    return 0;
  }
  return g_SCANNER.current[1];
}

static void skipWhitespace() {
  for (;;) {
    char c = peek();
    switch (c) {
      case ' ':
      case '\r':
      case '\t':
        advance();
        break;
      case '\n':
        g_SCANNER.line++;
        advance();
        break;
      case '/':
        if (peekNext() == '/') {
          while (peek() != '\n' && !isAtEnd()) {
            advance();
          }
        } else {
          return;
        }
        break;
      default:
        return;
    }
  }
}

static Token string() {
  while (peek() != '"' && !isAtEnd()) {
    if (peek() == '\n') {
      g_SCANNER.line++;
    }
    advance();
  }

  if (isAtEnd()) {
    return errorToken("Unterminated string.");
  }

  advance();
  return makeToken(TOKEN_STRING);
}

static bool isDigit(char c) {
  return c >= '0' && c <= '9';
}

static Token number() {
  while (isDigit(peek())) {
    advance();
  }

  // fractional part
  if (peek() == '.' && isDigit(peekNext())) {
    // the '.'
    advance();
    while (isDigit(peek())) {
      advance();
    }
  }

  return makeToken(TOKEN_NUMBER);
}

void initScanner(const char source[static 1]) {
  g_SCANNER.start = source;
  g_SCANNER.current = source;
  g_SCANNER.line = 1;
}

Token scanToken(void) {
  skipWhitespace();
  g_SCANNER.start = g_SCANNER.current;

  if (isAtEnd()) {
    return makeToken(TOKEN_EOF);
  }

  char c = advance();
  if (isDigit(c)) {
    return number();
  }

  switch (c) {
    case '(':
      return makeToken(TOKEN_LEFT_PAREN);
    case ')':
      return makeToken(TOKEN_RIGHT_PAREN);
    case '{':
      return makeToken(TOKEN_LEFT_BRACE);
    case '}':
      return makeToken(TOKEN_RIGHT_BRACE);
    case ';':
      return makeToken(TOKEN_SEMICOLON);
    case ',':
      return makeToken(TOKEN_COMMA);
    case '.':
      return makeToken(TOKEN_DOT);
    case '-':
      return makeToken(TOKEN_MINUS);
    case '+':
      return makeToken(TOKEN_PLUS);
    case '/':
      return makeToken(TOKEN_SLASH);
    case '*':
      return makeToken(TOKEN_STAR);
    case '!':
      return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
    case '=':
      return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
    case '<':
      return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
    case '>':
      return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
    case '"':
      return string();
    default:
      break;
  }

  return errorToken("Unexpected character.");
}
