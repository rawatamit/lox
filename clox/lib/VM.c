#include "VM.h"
#include "Compiler.h"
#include "Debug.h"
#include "Opcode.h"
#include "Value.h"
#include "Object.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

static uint8_t read_byte(CallFrame *);
static uint16_t read_short(CallFrame *);
static Value read_constant(CallFrame *);
static ObjString *read_string(CallFrame *);
static InterpretResult run(VM *vm);
static void reset_stack(VM *vm);
static void runtime_error(VM *vm, const char *format, ...);
static InterpretResult binary_op(VM *vm, Value (*fn)(Value, Value));

void init_VM(VM *vm)
{
  reset_stack(vm);
  init_table(&vm->strings);
  init_table(&vm->globals);
  define_native(vm, "clock", clock_native);
  vm->objects = NULL;
}

void free_VM(VM *vm)
{
  free_table(&vm->strings);
  free_table(&vm->globals);

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
  ObjFunction *fn = compile(vm, src);
  if (fn == NULL)
  {
    return INTERPRET_COMPILE_ERROR;
  }

  push(vm, object_val((Obj *)fn));
  ObjClosure *closure = new_closure(vm, fn);
  pop(vm);
  push(vm, object_val((Obj *)closure));
  call_value(vm, object_val((Obj *)closure), 0);
  return run(vm);
}

void push(VM *vm, Value value) { *vm->stack_top++ = value; }

Value pop(VM *vm) { return *--vm->stack_top; }

Value peek(VM *vm, size_t index)
{
  return vm->stack_top[-1 - index];
}

static bool call(VM *vm, ObjClosure *closure, int arg_count)
{
  if (arg_count != closure->fn->arity)
  {
    runtime_error(vm, "Expected %d arguments but got %d.",
                  closure->fn->arity, arg_count);
    return false;
  }

  if (vm->frame_count == FRAMES_MAX)
  {
    runtime_error(vm, "Stack overflow.");
    return false;
  }

  CallFrame *frame = &vm->frames[vm->frame_count++];
  frame->closure = closure;
  frame->ip = closure->fn->chunk.code;
  frame->slots = vm->stack_top - arg_count - 1;
  return true;
}

bool call_value(VM *vm, Value callee, int arg_count)
{
  if (is_object(callee))
  {
    switch (object_type(callee))
    {
    case OBJ_CLOSURE:
      return call(vm, as_closure(callee), arg_count);

    case OBJ_NATIVE:
    {
      NativeFn fn = as_native(callee);
      Value res = fn(arg_count, vm->stack_top - arg_count);
      vm->stack_top -= arg_count + 1;
      push(vm, res);
      return true;
    }

    default:
      break;
    }
  }

  runtime_error(vm, "Can only call functions and classes.");
  return false;
}

InterpretResult run(VM *vm)
{
  CallFrame *frame = &vm->frames[vm->frame_count - 1];

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
    disassemble_instruction(&frame->closure->fn->chunk, (size_t)(frame->ip - frame->closure->fn->chunk.code));
#endif

    uint8_t inst = read_byte(frame);

    switch (inst)
    {
    case OP_CLOSURE:
    {
      ObjFunction *fn = as_function(read_constant(frame));
      ObjClosure *closure = new_closure(vm, fn);
      push(vm, object_val((Obj *)closure));
      break;
    }

    case OP_DEFINE_GLOBAL:
    {
      ObjString *name = read_string(frame);
      table_set(&vm->globals, name, peek(vm, 0));
      pop(vm);
      break;
    }

    case OP_GET_GLOBAL:
    {
      ObjString *name = read_string(frame);
      Value value;
      if (!table_get(&vm->globals, name, &value))
      {
        runtime_error(vm, "Undefined variable '%s'.", name->chars);
        return INTERPRET_RUNTIME_ERROR;
      }

      push(vm, value);
      break;
    }

    case OP_SET_GLOBAL:
    {
      ObjString *name = read_string(frame);
      if (table_set(&vm->globals, name, peek(vm, 0)))
      {
        table_delete(&vm->globals, name);
        runtime_error(vm, "Undefined variable '%s'.", name->chars);
        return INTERPRET_RUNTIME_ERROR;
      }
      break;
    }

    case OP_GET_LOCAL:
    {
      uint8_t slot = read_byte(frame);
      push(vm, frame->slots[slot]);
      break;
    }

    case OP_SET_LOCAL:
    {
      uint8_t slot = read_byte(frame);
      frame->slots[slot] = peek(vm, 0);
      break;
    }

    case OP_PRINT:
      print_value(pop(vm));
      putchar('\n');
      break;

    case OP_LOOP:
    {
      uint16_t offset = read_short(frame);
      frame->ip -= offset;
      break;
    }

    case OP_JUMP:
    {
      uint16_t offset = read_short(frame);
      frame->ip += offset;
      break;
    }

    case OP_JUMP_IF_FALSE:
    {
      uint16_t offset = read_short(frame);
      if (is_falsey(peek(vm, 0)))
      {
        frame->ip += offset;
      }
      break;
    }

    case OP_CALL:
    {
      int arg_count = read_byte(frame);
      if (!call_value(vm, peek(vm, arg_count), arg_count))
      {
        return INTERPRET_RUNTIME_ERROR;
      }
      frame = &vm->frames[vm->frame_count - 1];
      break;
    }

    case OP_RETURN:
    {
      Value res = pop(vm);
      --vm->frame_count;
      if (vm->frame_count == 0)
      {
        pop(vm);
        return INTERPRET_OK;
      }
      else
      {
        vm->stack_top = frame->slots;
        push(vm, res);
        frame = &vm->frames[vm->frame_count - 1];
        break;
      }
    }

    case OP_CONSTANT:
    {
      Value constant = read_constant(frame);
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

    case OP_POP:
      pop(vm);
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

uint8_t read_byte(CallFrame *frame) { return *frame->ip++; }

uint16_t read_short(CallFrame *frame)
{
  frame->ip += 2;
  return (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]);
}

Value read_constant(CallFrame *frame)
{
  uint8_t index = read_byte(frame);
  return frame->closure->fn->chunk.constants.values[index];
}

ObjString *read_string(CallFrame *frame)
{
  return as_string(read_constant(frame));
}

void reset_stack(VM *vm)
{
  vm->stack_top = vm->stack;
  vm->frame_count = 0;
}

static void runtime_error(VM *vm, const char *format, ...)
{
  va_list args;
  va_start(args, format);
  vfprintf(stdout, format, args);
  va_end(args);
  putc('\n', stderr);

  for (int i = vm->frame_count - 1; i >= 0; i--)
  {
    CallFrame *frame = &vm->frames[i];
    ObjFunction *function = frame->closure->fn;
    // -1 because the IP is sitting on the next instruction to be
    // executed.
    size_t instruction = frame->ip - function->chunk.code - 1;
    fprintf(stderr, "[line %d] in ",
            function->chunk.lines[instruction]);
    if (function->name == NULL)
    {
      fprintf(stderr, "script\n");
    }
    else
    {
      fprintf(stderr, "%s()\n", function->name->chars);
    }
  }

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

void define_native(VM *vm, const char *name, NativeFn fn)
{
  push(vm, object_val((Obj *)copy_string(vm, name, strlen(name))));
  push(vm, object_val((Obj *)new_native(vm, fn)));
  table_set(&vm->globals, as_string(vm->stack[0]), vm->stack[1]);
  pop(vm);
  pop(vm);
}

Value clock_native(int arg_count, Value *args)
{
  return number_val((double)clock() / CLOCKS_PER_SEC);
}
