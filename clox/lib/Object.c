#include "Object.h"
#include "Memory.h"
#include "Table.h"
#include "VM.h"
#include <stdio.h>
#include <string.h>

ObjType object_type(Value value) { return as_object(value)->type; }

ObjFunction *new_function(VM *vm) {
  ObjFunction *fn =
      (ObjFunction *)allocate_object(vm, sizeof(ObjFunction), OBJ_FUNCTION);
  fn->arity = 0;
  fn->upvalue_count = 0;
  fn->name = NULL;
  init_chunk(&fn->chunk);
  return fn;
}

ObjNative *new_native(VM *vm, NativeFn fn) {
  ObjNative *native_fn =
      (ObjNative *)allocate_object(vm, sizeof(ObjNative), OBJ_NATIVE);
  native_fn->fn = fn;
  return native_fn;
}

ObjClosure *new_closure(VM *vm, ObjFunction *fn) {
  ObjUpvalue **upvalues = allocate(vm, sizeof(ObjUpvalue *), fn->upvalue_count);
  for (int i = 0; i < fn->upvalue_count; ++i) {
    upvalues[i] = NULL;
  }

  ObjClosure *closure =
      (ObjClosure *)allocate_object(vm, sizeof(ObjClosure), OBJ_CLOSURE);
  closure->fn = fn;
  closure->upvalues = upvalues;
  closure->upvalue_count = fn->upvalue_count;
  return closure;
}

ObjUpvalue *new_upvalue(VM *vm, Value *slot) {
  ObjUpvalue *upvalue =
      (ObjUpvalue *)allocate_object(vm, sizeof(ObjUpvalue), OBJ_UPVALUE);
  upvalue->closed = nil_val();
  upvalue->location = slot;
  upvalue->next = NULL;
  return upvalue;
}

ObjString *copy_string(VM *vm, const char *chars, size_t length) {
  uint32_t hash = hash_string(chars, length);
  ObjString *interned = table_find_string(&vm->strings, chars, length, hash);
  if (interned != NULL)
    return interned;
  char *value = allocate(vm, sizeof(char), length + 1);
  memcpy(value, chars, length);
  value[length] = '\0';
  return allocate_string(vm, value, length, hash);
}

ObjString *allocate_string(VM *vm, char *chars, size_t length, uint32_t hash) {
  ObjString *string =
      (ObjString *)allocate_object(vm, sizeof(ObjString), OBJ_STRING);
  string->chars = chars;
  string->length = length;
  string->hash = hash;
  push(vm, object_val((Obj *)string));
  table_set(vm, &vm->strings, string, nil_val());
  pop(vm);
  return string;
}

Value concatenate(VM *vm, ObjString *sa, ObjString *sb) {
  int length = sa->length + sb->length;
  char *chars = allocate(vm, sizeof(char), length);

  memcpy(chars, sa->chars, sa->length);
  memcpy(chars + sa->length, sb->chars, sb->length);
  chars[length] = '\0';

  ObjString *sobj = take_string(vm, chars, length);
  return object_val((Obj *)sobj);
}

ObjString *take_string(VM *vm, char *chars, int length) {
  uint32_t hash = hash_string(chars, length);
  ObjString *interned = table_find_string(&vm->strings, chars, length, hash);
  if (interned != NULL) {
    free_array(vm, sizeof(char), chars, length);
    return interned;
  }

  return allocate_string(vm, chars, length, hash);
}

void print_object(FILE *out, Value value) {
  switch (object_type(value)) {
  case OBJ_FUNCTION:
    print_function(out, as_function(value));
    break;

  case OBJ_NATIVE:
    fprintf(out, "<native fn>");
    break;

  case OBJ_CLOSURE:
    print_function(out, as_closure(value)->fn);
    break;

  case OBJ_UPVALUE:
    fprintf(out, "upvalue");
    break;

  case OBJ_STRING:
    fprintf(out, "%s", as_cstring(value));
    break;

  default:
    break;
  }
}

void print_function(FILE *out, ObjFunction *fn) {
  if (fn->name != NULL) {
    fprintf(out, "<fn %s>", fn->name->chars);
  } else {
    fprintf(out, "<fn ||between-allocation||>");
  }
}

bool is_equal_object(Obj *obja, Obj *objb) {
  switch (obja->type) {
  case OBJ_STRING:
    return obja == objb;

  default:
    return false;
  }
}

bool is_object_type(Value value, ObjType type) {
  return object_type(value) == type;
}

bool is_string(Value value) {
  return is_object(value) && is_object_type(value, OBJ_STRING);
}

bool is_function(Value value) {
  return is_object(value) && is_object_type(value, OBJ_FUNCTION);
}

bool is_native(Value value) {
  return is_object(value) && is_object_type(value, OBJ_NATIVE);
}

bool is_closure(Value value) {
  return is_object(value) && is_object_type(value, OBJ_CLOSURE);
}

ObjString *as_string(Value value) { return (ObjString *)as_object(value); }

ObjFunction *as_function(Value value) {
  return (ObjFunction *)as_object(value);
}

NativeFn as_native(Value value) { return ((ObjNative *)as_object(value))->fn; }

ObjClosure *as_closure(Value value) { return (ObjClosure *)as_object(value); }

char *as_cstring(Value value) { return ((ObjString *)as_object(value))->chars; }

uint32_t hash_string(const char *key, int length) {
  uint32_t hash = 2166136261u;

  for (int i = 0; i < length; i++) {
    hash ^= key[i];
    hash *= 16777619;
  }

  return hash;
}
