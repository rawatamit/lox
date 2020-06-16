#include "Debug.h"
#include "Opcode.h"
#include "Value.h"
#include <stdio.h>

static size_t simple_instruction(const char *name, size_t offset);
static size_t constant_instruction(const char *name, Chunk *chunk,
                                   size_t offset);
static void print_value(Value value);

void disassemble_chunk(Chunk *chunk, const char *name) {
  fprintf(stderr, "== %s ==\n", name);

  for (size_t offset = 0; offset < chunk->size;) {
    offset = disassemble_instruction(chunk, offset);
  }
}

size_t disassemble_instruction(Chunk *chunk, size_t offset) {
  fprintf(stderr, "%04ld ", offset);

  if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1]) {
    fprintf(stderr, "   | ");
  } else {
    fprintf(stderr, "%4d ", chunk->lines[offset]);
  }

  uint8_t inst = chunk->code[offset];
  switch (inst) {
  case OP_RETURN:
    return simple_instruction("OP_RETURN", offset);
  case OP_CONSTANT:
    return constant_instruction("OP_CONSTANT", chunk, offset);
  case OP_NEGATE:
    return simple_instruction("OP_NEGATE", offset);
  case OP_ADD:
    return simple_instruction("OP_ADD", offset);
  case OP_SUBTRACT:
    return simple_instruction("OP_SUBTRACT", offset);
  case OP_MULTIPLY:
    return simple_instruction("OP_MULTIPLY", offset);
  case OP_DIVIDE:
    return simple_instruction("OP_DIVIDE", offset);
  default:
    fprintf(stderr, "Unknown opcode %d\n", inst);
    return offset + 1;
  }
}

size_t simple_instruction(const char *name, size_t offset) {
  fprintf(stderr, "%s\n", name);
  return offset + 1;
}

size_t constant_instruction(const char *name, Chunk *chunk, size_t offset) {
  uint8_t constant = chunk->code[offset + 1];
  fprintf(stderr, "%-16s %4d '", name, constant);
  print_value(chunk->constants.values[constant]);
  fprintf(stderr, "'\n");
  return offset + 2;
}

void print_value(Value value) { fprintf(stderr, "%lf", value); }
