#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "Value.h"

typedef struct VM VM;

enum ObjType {
  OBJ_STRING,
  OBJ_FUNCTION,
};

typedef enum ObjType ObjType;

struct Obj {
  ObjType type;
  struct Obj *next;
};

typedef struct Obj Obj;

struct ObjString {
  Obj obj;
  size_t length;
  char *chars;
};

typedef struct ObjString ObjString;

void free_object(Obj *obj);
ObjType object_type(Value value);
ObjString *copy_string(VM *vm, const char *chars, size_t length);
Value concatenate(VM *vm, ObjString *sa, ObjString *sb);
ObjString *take_string(VM *vm, char *chars, int length);
ObjString *allocate_string(VM *vm, char *chars, size_t length);
void print_object(Value value);

bool is_equal_object(Obj *obja, Obj *objb);
bool is_object_type(Value value, ObjType type);
bool is_string(Value value);

ObjString *as_string(Value value);
char *as_cstring(Value value);

#endif
