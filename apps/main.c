#include <clox/chunk.h>
#include <clox/debug.h>
#include <clox/vm.h>

int main(void) {
  initVm();

  Chunk chunk;
  initChunk(&chunk);

  writeConstant(&chunk, 1.2, 123);
  for (int i = 0; i < 260; ++i)
  {
    writeConstant(&chunk, i, 123);
  }
  writeChunk(&chunk, OP_RETURN, 123);

  disassembleChunk(&chunk, "test chunk");
  interpret(&chunk);
  freeVm();
  freeChunk(&chunk);
  return 0;
}
