#ifndef _CHUNK_H_
#define _CHUNK_H_

#include "Value.h"
#include <stddef.h>
#include <stdint.h>

struct Chunk {
  size_t size;
  size_t capacity;
  uint8_t *code;
  int *lines;
  ValueArray constants;
};

typedef struct Chunk Chunk;

void init_chunk(Chunk *chunk);
void free_chunk(Chunk *chunk);
void write_chunk(Chunk *chunk, uint8_t byte, int line);
size_t add_constant(Chunk *chunk, Value value);

#endif
