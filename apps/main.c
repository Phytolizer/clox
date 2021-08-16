#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <clox/chunk.h>
#include <clox/debug.h>
#include <clox/vm.h>
#include <sysexits.h>

static void repl() {
  char line[1024];
  for (;;) {
    fputs("> ", stdout);
    fflush(stdout);

    if (!fgets(line, sizeof(line), stdin)) {
      fputs("\n", stdout);
      break;
    }

    interpret(line);
  }
}

typedef enum read_result_type_e
{
  READ_RESULT_TYPE_OK,
  READ_RESULT_TYPE_ERR,
} ReadResultType;

typedef struct read_result_s {
  ReadResultType type;
  union read_result_u {
    char* ok;
    int err;
  } u;
} ReadResult;

#define READ_OK(ok_val) \
  (ReadResult) { \
    .type = READ_RESULT_TYPE_OK, .u = {.ok = (ok_val) } \
  }
#define READ_ERR(err_val) \
  (ReadResult) { \
    .type = READ_RESULT_TYPE_ERR, .u = {.err = (err_val) } \
  }

#define READ_IS_OK(res) ((res).type == READ_RESULT_TYPE_OK)
#define READ_IS_ERR(res) ((res).type == READ_RESULT_TYPE_ERR)
#define READ_GET_OK(res) ((res).u.ok)
#define READ_GET_ERR(res) ((res).u.err)

static ReadResult readFile(const char path[static 1]) {
  FILE* file = fopen(path, "rbe");
  if (!file) {
    fprintf(stderr, "Could not open file \"%s\".\n", path);
    return READ_ERR(EX_IOERR);
  }

  fseek(file, 0, SEEK_END);
  size_t fileSize = ftell(file);
  rewind(file);

  char* buffer = (char*)malloc(fileSize + 1);
  if (!buffer) {
    fclose(file);
    fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
    return READ_ERR(EX_IOERR);
  }
  size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
  if (bytesRead < fileSize) {
    fprintf(stderr, "Could not read file \"%s\".\n", path);
    return READ_ERR(EX_IOERR);
  }
  buffer[bytesRead] = 0;

  fclose(file);
  return READ_OK(buffer);
}

static int runFile(const char path[static 1]) {
  ReadResult readResult = readFile(path);
  if (READ_IS_ERR(readResult)) {
    return READ_GET_ERR(readResult);
  }
  char* source = READ_GET_OK(readResult);
  InterpretResult result = interpret(source);
  free(source);

  if (result == INTERPRET_COMPILE_ERROR) {
    return EX_DATAERR;
  }
  if (result == INTERPRET_RUNTIME_ERROR) {
    return EX_SOFTWARE;
  }
  return EXIT_SUCCESS;
}

int main(int argc, const char* argv[argc + 1]) {
  initVm();

  int ret = EXIT_SUCCESS;

  switch (argc) {
    case 1:
      repl();
      break;
    case 2:
      ret = runFile(argv[1]);
      break;
    default:
      fputs("Usage: clox [path]\n", stderr);
      ret = EX_USAGE;
  }

  freeVm();
  return ret;
}
