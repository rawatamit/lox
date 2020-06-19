#ifndef _VM_H_
#define _VM_H_

#include "Chunk.h"
#include "InterpretResult.h"
#include "Object.h"
#include "Table.h"

#define STACK_MAX 256

struct VM {
  Chunk *chunk;
  uint8_t *ip;
  Value stack[STACK_MAX];
  Value *stack_top;
  Obj *objects;
  Table strings;
  Table globals;
};

typedef struct VM VM;

void init_VM(VM *vm);
void free_VM(VM *vm);
InterpretResult interpret(VM *vm, const char *src);
void push(VM *vm, Value value);
Value pop(VM *vm);
Value peek(VM *vm, size_t index);

#endif
