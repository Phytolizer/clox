#include <clox/chunk.h>
#include <clox/debug.h>
#include <clox/vm.h>

int main(void) {
  initVm();

  Chunk chunk;
  initChunk(&chunk);

  writeConstant(&chunk, 1.2, 123);
  writeChunk(&chunk, OP_NEGATE, 123);
  writeChunk(&chunk, OP_RETURN, 123);

  disassembleChunk(&chunk, "test chunk");
  interpret(&chunk);
  freeVm();
  freeChunk(&chunk);
  return 0;
}
