#include "VM.h"
#include "Compiler.h"
#include "Debug.h"
#include "Opcode.h"
#include "Value.h"
#include "Object.h"
#include <stdio.h>
#include <stdarg.h>

static uint8_t read_byte(VM *vm);
static Value read_constant(VM *vm);
static void print_value(Value constant);
static InterpretResult run(VM *vm);
static void reset_stack(VM *vm);
static void runtime_error(VM *vm, const char *format, ...);
static InterpretResult binary_op(VM *vm, Value (*fn)(Value, Value));

void init_VM(VM *vm)
{
  reset_stack(vm);
  vm->objects = NULL;
}

void free_VM(VM *vm)
{
  Obj *object = vm->objects;

  while (object != NULL)
  {
    Obj *next = object->next;
    free_object(object);
    object = next;
  }
}

InterpretResult interpret(VM *vm, const char *src)
{
  Chunk chunk;
  init_chunk(&chunk);

  if (!compile(vm, src, &chunk))
  {
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

Value peek(VM *vm, size_t index)
{
  return vm->stack_top[-1 - index];
}

InterpretResult run(VM *vm)
{
  for (;;)
  {
#ifdef DEBUG_TRACE_EXECUTION
    printf("      ");
    for (Value *slot = vm->stack; slot < vm->stack_top; ++slot)
    {
      printf("[ ");
      print_value(*slot);
      printf(" ]");
    }
    putchar('\n');
    disassemble_instruction(vm->chunk, (size_t)(vm->ip - vm->chunk->code));
#endif

    uint8_t inst = read_byte(vm);

    switch (inst)
    {
    case OP_RETURN:
      print_value(pop(vm));
      printf("\n");
      return INTERPRET_OK;

    case OP_CONSTANT:
    {
      Value constant = read_constant(vm);
      push(vm, constant);
      break;
    }

    case OP_NEGATE:
    {
      if (!is_number(peek(vm, 0)))
      {
        runtime_error(vm, "Operand must be a number.");
        return INTERPRET_RUNTIME_ERROR;
      }

      double constant = -as_number(pop(vm));
      push(vm, number_val(constant));
      break;
    }

    case OP_NOT:
      push(vm, bool_val(is_falsey(pop(vm))));
      break;

    case OP_ADD:
      if (is_string(peek(vm, 0)) && is_string(peek(vm, 1)))
      {
        ObjString *sb = (ObjString *)as_object(pop(vm));
        ObjString *sa = (ObjString *)as_object(pop(vm));
        push(vm, concatenate(vm, sa, sb));
        break;
      }
      else if (is_number(peek(vm, 0)) && is_number(peek(vm, 1)))
      {
        if (binary_op(vm, add) == INTERPRET_RUNTIME_ERROR)
        {
          return INTERPRET_RUNTIME_ERROR;
        }
      }
      else
      {
        runtime_error(vm, "Operands must be two numbers or two strings.");
        return INTERPRET_RUNTIME_ERROR;
      }
      break;

    case OP_SUBTRACT:
      if (binary_op(vm, subtract) == INTERPRET_RUNTIME_ERROR)
      {
        return INTERPRET_RUNTIME_ERROR;
      }
      break;

    case OP_MULTIPLY:
      if (binary_op(vm, multiply) == INTERPRET_RUNTIME_ERROR)
      {
        return INTERPRET_RUNTIME_ERROR;
      }
      break;

    case OP_DIVIDE:
      if (binary_op(vm, divide) == INTERPRET_RUNTIME_ERROR)
      {
        return INTERPRET_RUNTIME_ERROR;
      }
      break;

    case OP_NIL:
      push(vm, nil_val());
      break;

    case OP_TRUE:
      push(vm, bool_val(true));
      break;

    case OP_FALSE:
      push(vm, bool_val(false));
      break;

    case OP_EQUAL:
    {
      Value b = pop(vm);
      Value a = pop(vm);
      push(vm, bool_val(is_equal(a, b)));
      break;
    }

    case OP_GREATER:
      if (binary_op(vm, greater) == INTERPRET_RUNTIME_ERROR)
      {
        return INTERPRET_RUNTIME_ERROR;
      }
      break;

    case OP_LESS:
      if (binary_op(vm, less) == INTERPRET_RUNTIME_ERROR)
      {
        return INTERPRET_RUNTIME_ERROR;
      }
      break;

    default:
      break;
    }
  }
}

uint8_t read_byte(VM *vm) { return *vm->ip++; }

Value read_constant(VM *vm)
{
  uint8_t index = read_byte(vm);
  return vm->chunk->constants.values[index];
}

void print_value(Value constant) { printf("%lf", as_number(constant)); }

void reset_stack(VM *vm) { vm->stack_top = vm->stack; }

static void runtime_error(VM *vm, const char *format, ...)
{
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  size_t instruction = vm->ip - vm->chunk->code - 1;
  int line = vm->chunk->lines[instruction];
  fprintf(stderr, "[line %d] in script\n", line);

  reset_stack(vm);
}

InterpretResult binary_op(VM *vm, Value (*fn)(Value, Value))
{
  if (!is_number(peek(vm, 0)) && !is_number(peek(vm, 1)))
  {
    runtime_error(vm, "Operands must be numbers.");
    return INTERPRET_RUNTIME_ERROR;
  }

  Value b = pop(vm);
  Value a = pop(vm);
  push(vm, fn(a, b));
  return INTERPRET_OK;
}
