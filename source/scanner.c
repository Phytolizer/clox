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

static bool isAlpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
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

static TokenType checkKeyword(int start,
    int length,
    const char rest[length],
    TokenType type) {
  if (g_SCANNER.current - g_SCANNER.start == start + length
      && memcmp(g_SCANNER.start + start, rest, length) == 0) {
    return type;
  }
  return TOKEN_IDENTIFIER;
}

static TokenType identifierType() {
  switch (g_SCANNER.start[0]) {
    case 'a':
      return checkKeyword(1, 2, "nd", TOKEN_AND);
    case 'c':
      return checkKeyword(1, 4, "lass", TOKEN_CLASS);
    case 'e':
      return checkKeyword(1, 3, "lse", TOKEN_ELSE);
    case 'f':
      if (g_SCANNER.current - g_SCANNER.start > 1) {
        switch (g_SCANNER.start[1]) {
          case 'a':
            return checkKeyword(2, 3, "lse", TOKEN_FALSE);
          case 'o':
            return checkKeyword(2, 1, "r", TOKEN_FOR);
          case 'u':
            return checkKeyword(2, 1, "n", TOKEN_FUN);
        }
      }
    case 'i':
      return checkKeyword(1, 1, "f", TOKEN_IF);
    case 'n':
      return checkKeyword(1, 2, "il", TOKEN_NIL);
    case 'o':
      return checkKeyword(1, 1, "r", TOKEN_OR);
    case 'p':
      return checkKeyword(1, 4, "rint", TOKEN_PRINT);
    case 'r':
      return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
    case 's':
      return checkKeyword(1, 4, "uper", TOKEN_SUPER);
    case 't':
      if (g_SCANNER.current - g_SCANNER.start > 1) {
        switch (g_SCANNER.start[1]) {
          case 'h':
            return checkKeyword(2, 2, "is", TOKEN_THIS);
          case 'r':
            return checkKeyword(2, 2, "ue", TOKEN_TRUE);
        }
      }
    case 'v':
      return checkKeyword(1, 2, "ar", TOKEN_VAR);
    case 'w':
      return checkKeyword(1, 4, "hile", TOKEN_WHILE);
  }
}

static Token identifier() {
  while (isAlpha(peek()) || isDigit(peek())) {
    advance();
  }
  return makeToken(identifierType());
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
  if (isAlpha(c)) {
    return identifier();
  }
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
