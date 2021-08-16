//
// Created by kyle on 8/15/21.
//

#include <stdio.h>

#include <clox/common.h>
#include <clox/compiler.h>
#include <clox/scanner.h>

void compile(const char source[static 1]) {
  initScanner(source);
  int line = -1;
  for (;;) {
    Token token = scanToken();
    if (token.line != line) {
      printf("%4d ", token.line);
      line = token.line;
    } else {
      fputs("   | ", stdout);
    }
    printf("%-20s '%.*s'\n",
        TOKEN_NAMES[token.type],
        token.length,
        token.start);

    if (token.type == TOKEN_EOF) {
      break;
    }
  }
}
