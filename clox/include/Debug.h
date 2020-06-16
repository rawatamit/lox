#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "Chunk.h"

void disassemble_chunk(Chunk *chunk, const char *name);
size_t disassemble_instruction(Chunk *chunk, size_t offset);

#endif
