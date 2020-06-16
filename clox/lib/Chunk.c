#include "Chunk.h"
#include "Memory.h"
#include <stdlib.h>

void init_chunk(Chunk *chunk) {
  chunk->size = 0;
  chunk->capacity = 0;
  chunk->code = NULL;
  chunk->lines = NULL;
  init_value_array(&chunk->constants);
}

void free_chunk(Chunk *chunk) {
  free_array(sizeof(uint8_t), chunk->code, chunk->capacity);
  free_array(sizeof(int), chunk->lines, chunk->capacity);
  free_value_array(&chunk->constants);
  init_chunk(chunk);
}

void write_chunk(Chunk *chunk, uint8_t byte, int line) {
  if (chunk->capacity <= chunk->size) {
    size_t old_capacity = chunk->capacity;
    chunk->capacity = grow_capacity(old_capacity);
    chunk->code =
        grow_array(chunk->code, sizeof(uint8_t), old_capacity, chunk->capacity);
    chunk->lines =
        grow_array(chunk->lines, sizeof(int), old_capacity, chunk->capacity);
  }

  chunk->code[chunk->size] = byte;
  chunk->lines[chunk->size] = line;
  ++chunk->size;
}

size_t add_constant(Chunk *chunk, Value value) {
  write_value_array(&chunk->constants, value);
  return chunk->constants.size - 1;
}
