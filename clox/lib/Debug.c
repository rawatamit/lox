#include "Debug.h"
#include "Object.h"
#include "Opcode.h"
#include "Value.h"
#include <stdio.h>

void disassemble_chunk(Chunk *chunk, const char *name, FILE *out)
{
  fprintf(out, "== %s ==\n", name);

  for (size_t offset = 0; offset < chunk->size;)
  {
    offset = disassemble_instruction(chunk, offset, out);
  }
}

size_t disassemble_instruction(Chunk *chunk, size_t offset, FILE *out)
{
  fprintf(out, "%04ld ", offset);

  if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1])
  {
    fprintf(out, "   | ");
  }
  else
  {
    fprintf(out, "%4d ", chunk->lines[offset]);
  }

  uint8_t inst = chunk->code[offset];
  switch (inst)
  {
  case OP_CLASS:
    return constant_instruction("OP_CLASS", chunk, offset, out);
  case OP_GET_PROPERTY:
    return constant_instruction("OP_GET_PROPERTY", chunk, offset, out);
  case OP_SET_PROPERTY:
    return constant_instruction("OP_SET_PROPERTY", chunk, offset, out);
  case OP_METHOD:
    return constant_instruction("OP_METHOD", chunk, offset, out);
  case OP_CLOSURE:
  {
    ++offset;
    uint8_t constant = chunk->code[offset++];
    fprintf(out, "%-16s %4d ", "OP_CLOSURE", constant);
    print_value(out, chunk->constants.values[constant]);
    fputc('\n', out);

    ObjFunction *function = as_function(chunk->constants.values[constant]);
    for (int j = 0; j < function->upvalue_count; ++j)
    {
      int is_local = chunk->code[offset++];
      int index = chunk->code[offset++];
      fprintf(out, "%04ld      |                     %s %d\n", offset - 2,
              is_local ? "local" : "upvalue", index);
    }
    return offset;
  }
  case OP_GET_UPVALUE:
    return byte_instruction("OP_GET_UPVALUE", chunk, offset, out);
  case OP_SET_UPVALUE:
    return byte_instruction("OP_SET_UPVALUE", chunk, offset, out);
  case OP_CLOSE_UPVALUE:
    return simple_instruction("OP_CLOSE_UPVALUE", offset, out);
  case OP_DEFINE_GLOBAL:
    return constant_instruction("OP_DEFINE_GLOBAL", chunk, offset, out);
  case OP_GET_GLOBAL:
    return constant_instruction("OP_GET_GLOBAL", chunk, offset, out);
  case OP_SET_GLOBAL:
    return constant_instruction("OP_SET_GLOBAL", chunk, offset, out);
  case OP_GET_LOCAL:
    return byte_instruction("OP_GET_LOCAL", chunk, offset, out);
  case OP_SET_LOCAL:
    return byte_instruction("OP_SET_LOCAL", chunk, offset, out);
  case OP_PRINT:
    return simple_instruction("OP_PRINT", offset, out);
  case OP_LOOP:
    return jump_instruction("OP_LOOP", -1, chunk, offset, out);
  case OP_JUMP:
    return jump_instruction("OP_JUMP", 1, chunk, offset, out);
  case OP_JUMP_IF_FALSE:
    return jump_instruction("OP_JUMP_IF_ELSE", 1, chunk, offset, out);
  case OP_CALL:
    return byte_instruction("OP_CALL", chunk, offset, out);
  case OP_RETURN:
    return simple_instruction("OP_RETURN", offset, out);
  case OP_CONSTANT:
    return constant_instruction("OP_CONSTANT", chunk, offset, out);
  case OP_NEGATE:
    return simple_instruction("OP_NEGATE", offset, out);
  case OP_ADD:
    return simple_instruction("OP_ADD", offset, out);
  case OP_SUBTRACT:
    return simple_instruction("OP_SUBTRACT", offset, out);
  case OP_MULTIPLY:
    return simple_instruction("OP_MULTIPLY", offset, out);
  case OP_DIVIDE:
    return simple_instruction("OP_DIVIDE", offset, out);
  case OP_NIL:
    return simple_instruction("OP_NIL", offset, out);
  case OP_TRUE:
    return simple_instruction("OP_TRUE", offset, out);
  case OP_FALSE:
    return simple_instruction("OP_FALSE", offset, out);
  case OP_NOT:
    return simple_instruction("OP_NOT", offset, out);
  case OP_POP:
    return simple_instruction("OP_POP", offset, out);
  case OP_EQUAL:
    return simple_instruction("OP_EQUAL", offset, out);
  case OP_GREATER:
    return simple_instruction("OP_GREATER", offset, out);
  case OP_LESS:
    return simple_instruction("OP_LESS", offset, out);
  default:
    fprintf(stderr, "Unknown opcode %d\n", inst);
    return offset + 1;
  }
}

size_t simple_instruction(const char *name, size_t offset, FILE *out)
{
  fprintf(out, "%s\n", name);
  return offset + 1;
}

size_t constant_instruction(const char *name, Chunk *chunk, size_t offset, FILE *out)
{
  uint8_t constant = chunk->code[offset + 1];
  fprintf(out, "%-16s %4d '", name, constant);
  print_value(out, chunk->constants.values[constant]);
  fprintf(out, "'\n");
  return offset + 2;
}

size_t byte_instruction(const char *name, Chunk *chunk, size_t offset, FILE *out)
{
  uint8_t slot = chunk->code[offset + 1];
  fprintf(out, "%-16s %4d\n", name, slot);
  return offset + 2;
}

size_t jump_instruction(const char *name, int sign, Chunk *chunk, int offset, FILE *out)
{
  uint16_t jump = (uint16_t)(chunk->code[offset + 1] << 8);
  jump |= chunk->code[offset + 2];
  fprintf(out, "%-16s %4d -> %d\n", name, offset, offset + 3 + sign * jump);
  return offset + 3;
}
