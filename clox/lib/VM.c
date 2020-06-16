#include "VM.h"
#include "Compiler.h"
#include "Debug.h"
#include "Opcode.h"
#include "Value.h"
#include <stdio.h>

static uint8_t read_byte(VM *vm);
static Value read_constant(VM *vm);
static void print_value(Value constant);
static InterpretResult run(VM *vm);
static void reset_stack(VM *vm);

void init_VM(VM *vm) { reset_stack(vm); }

void free_VM(VM *vm) { vm->chunk = NULL; }

InterpretResult interpret(VM *vm, const char *src) {
  Chunk chunk;
  init_chunk(&chunk);

  if (!compile(src, &chunk)) {
    free_chunk(&chunk);
    return INTERPRET_COMPILE_ERROR;
  }

  vm->chunk = &chunk;
  vm->ip = vm->chunk->code;

  InterpretResult res = run(vm);
  free_chunk(&chunk);
  return res;
}

void push(VM *vm, Value value) { *vm->stack_top++ = value; }

Value pop(VM *vm) { return *--vm->stack_top; }

InterpretResult run(VM *vm) {
  for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
    printf("      ");
    for (Value *slot = vm->stack; slot < vm->stack_top; ++slot) {
      printf("[ ");
      print_value(*slot);
      printf(" ]");
    }
    putchar('\n');
    disassemble_instruction(vm->chunk, (size_t)(vm->ip - vm->chunk->code));
#endif

    uint8_t inst = read_byte(vm);

    switch (inst) {
    case OP_RETURN:
      print_value(pop(vm));
      printf("\n");
      return INTERPRET_OK;

    case OP_CONSTANT: {
      Value constant = read_constant(vm);
      push(vm, constant);
      break;
    }

    case OP_NEGATE: {
      Value constant = -pop(vm);
      push(vm, constant);
      break;
    }

    case OP_ADD: {
      double b = pop(vm);
      double a = pop(vm);
      push(vm, a + b);
      break;
    }

    case OP_SUBTRACT: {
      double b = pop(vm);
      double a = pop(vm);
      push(vm, a - b);
      break;
    }

    case OP_MULTIPLY: {
      double b = pop(vm);
      double a = pop(vm);
      push(vm, a * b);
      break;
    }

    case OP_DIVIDE: {
      double b = pop(vm);
      double a = pop(vm);
      push(vm, a / b);
      break;
    }

    default:
      break;
    }
  }
}

uint8_t read_byte(VM *vm) { return *vm->ip++; }

Value read_constant(VM *vm) {
  uint8_t index = read_byte(vm);
  return vm->chunk->constants.values[index];
}

void print_value(Value constant) { printf("%lf", constant); }

void reset_stack(VM *vm) { vm->stack_top = vm->stack; }
