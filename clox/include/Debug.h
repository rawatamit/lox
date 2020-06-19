#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "Chunk.h"

void disassemble_chunk(Chunk *chunk, const char *name);
size_t disassemble_instruction(Chunk *chunk, size_t offset);
size_t simple_instruction(const char *name, size_t offset);
size_t constant_instruction(const char *name, Chunk *chunk, size_t offset);
size_t byte_instruction(const char *name, Chunk *chunk, size_t offset);
size_t jump_instruction(const char *name, int sign, Chunk *chunk, int offset);

#endif
