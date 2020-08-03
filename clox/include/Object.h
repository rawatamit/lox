#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "Chunk.h"
#include "Table.h"
#include "Value.h"
#include <stdint.h>
#include <stdio.h>

typedef struct VM VM;

enum ObjType {
  OBJ_STRING,
  OBJ_FUNCTION,
  OBJ_NATIVE,
  OBJ_CLOSURE,
  OBJ_UPVALUE,
  OBJ_CLASS,
  OBJ_INSTANCE,
  OBJ_METHOD,
  OBJ_BOUND_METHOD,
};

typedef enum ObjType ObjType;

struct Obj {
  ObjType type;
  bool is_marked;
  struct Obj *next;
};

typedef struct Obj Obj;

struct ObjString {
  Obj obj;
  size_t length;
  char *chars;
  uint32_t hash;
};

typedef struct ObjString ObjString;

struct ObjFunction {
  Obj obj;
  int arity;
  int upvalue_count;
  Chunk chunk;
  ObjString *name;
};

typedef struct ObjFunction ObjFunction;

typedef Value (*NativeFn)(int arg_count, Value *args);

struct ObjNative {
  Obj obj;
  NativeFn fn;
};

typedef struct ObjNative ObjNative;

struct ObjUpvalue {
  Obj obj;
  Value *location;
  Value closed;
  struct ObjUpvalue *next;
};

typedef struct ObjUpvalue ObjUpvalue;

struct ObjClosure {
  Obj obj;
  ObjFunction *fn;
  ObjUpvalue **upvalues;
  int upvalue_count;
};

typedef struct ObjClosure ObjClosure;

struct ObjClass {
  Obj obj;
  ObjString *name;
  Table methods;
};

typedef struct ObjClass ObjClass;

struct ObjInstance {
  Obj obj;
  ObjClass *klass;
  Table fields;
};

typedef struct ObjInstance ObjInstance;

struct ObjBoundMethod {
  Obj obj;
  Value receiver;
  ObjClosure *method;
};

typedef struct ObjBoundMethod ObjBoundMethod;

ObjType object_type(Value value);
ObjFunction *new_function(VM *vm);
ObjNative *new_native(VM *vm, NativeFn fn);
ObjClosure *new_closure(VM *vm, ObjFunction *fn);
ObjUpvalue *new_upvalue(VM *vm, Value *slot);
ObjClass *new_class(VM *vm, ObjString *name);
ObjInstance *new_instance(VM *vm, ObjClass *klass);
ObjBoundMethod *new_bound_method(VM *vm, Value receiver, ObjClosure *method);
ObjString *copy_string(VM *vm, const char *chars, size_t length);
void concatenate(VM *vm);
ObjString *take_string(VM *vm, char *chars, int length);
ObjString *allocate_string(VM *vm, char *chars, size_t length, uint32_t hash);
void print_object(FILE *out, Value value);
void print_function(FILE *out, ObjFunction *fn);

bool is_object_type(Value value, ObjType type);
bool is_string(Value value);
bool is_function(Value value);
bool is_native(Value value);
bool is_closure(Value value);
bool is_class(Value value);
bool is_instance(Value value);
bool is_bound_method(Value value);

ObjString *as_string(Value value);
ObjFunction *as_function(Value value);
NativeFn as_native(Value value);
ObjClosure *as_closure(Value value);
char *as_cstring(Value value);
ObjClass *as_class(Value value);
ObjInstance *as_instance(Value value);
ObjBoundMethod *as_bound_method(Value value);

uint32_t hash_string(const char *key, int length);

#endif
