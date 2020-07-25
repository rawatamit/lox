#ifndef _VM_H_
#define _VM_H_

#include "Chunk.h"
#include "InterpretResult.h"
#include "Object.h"
#include "Table.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * 256)

typedef struct Compiler Compiler;

struct CallFrame {
  ObjClosure *closure;
  uint8_t *ip;
  Value *slots;
};

typedef struct CallFrame CallFrame;

struct VM {
  // for GC
  Compiler *compiler;
  CallFrame frames[FRAMES_MAX];
  int frame_count;
  Value stack[STACK_MAX];
  Value *stack_top;
  Obj *objects;
  Table strings;
  Table globals;
  int gray_size;
  int gray_capacity;
  Obj **gray_stack;
  size_t bytes_allocated;
  size_t next_gc;
  ObjUpvalue *open_upvalues;
};

typedef struct VM VM;

void init_VM(VM *vm);
void free_VM(VM *vm);
InterpretResult interpret(VM *vm, const char *src);
void push(VM *vm, Value value);
Value pop(VM *vm);
Value peek(VM *vm, size_t index);
bool call_value(VM *vm, Value callee, int arg_count);

void define_native(VM *vm, const char *name, NativeFn fn);
Value clock_native(int arg_count, Value *args);

ObjUpvalue *capture_upvalue(VM *vm, Value *slot);
void close_upvalues(VM *vm, Value *last);

#endif
