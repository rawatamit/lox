#include "Chunk.h"
#include "Memory.h"
#include "VM.h"
#include <stdlib.h>

void init_chunk(Chunk *chunk)
{
  chunk->size = 0;
  chunk->capacity = 0;
  chunk->code = NULL;
  chunk->lines = NULL;
  init_value_array(&chunk->constants);
}

void free_chunk(VM *vm, Chunk *chunk)
{
  free_array(vm, sizeof(uint8_t), chunk->code, chunk->capacity);
  free_array(vm, sizeof(int), chunk->lines, chunk->capacity);
  free_value_array(vm, &chunk->constants);
  init_chunk(chunk);
}

void write_chunk(VM *vm, Chunk *chunk, uint8_t byte, int line)
{
  if (chunk->capacity <= chunk->size)
  {
    size_t old_capacity = chunk->capacity;
    chunk->capacity = grow_capacity(old_capacity);
    chunk->code =
        grow_array(vm, chunk->code, sizeof(uint8_t), old_capacity, chunk->capacity);
    chunk->lines =
        grow_array(vm, chunk->lines, sizeof(int), old_capacity, chunk->capacity);
  }

  chunk->code[chunk->size] = byte;
  chunk->lines[chunk->size] = line;
  ++chunk->size;
}

size_t add_constant(VM *vm, Chunk *chunk, Value value)
{
  push(vm, value);
  write_value_array(vm, &chunk->constants, value);
  pop(vm);
  return chunk->constants.size - 1;
}
