#include "Object.h"
#include "Memory.h"
#include "Table.h"
#include "VM.h"
#include <string.h>
#include <stdio.h>

void free_object(Obj *obj)
{
  switch (obj->type)
  {
  case OBJ_STRING:
  {
    ObjString *string = (ObjString *)obj;
    // free string
    free_array(sizeof(char), string->chars, string->length + 1);
    // free object
    reallocate(obj, sizeof(ObjString), 0);
    break;
  }

  default:
    break;
  }
}

ObjType object_type(Value value)
{
  return as_object(value)->type;
}

ObjString *copy_string(VM *vm, const char *chars, size_t length)
{
  uint32_t hash = hash_string(chars, length);
  ObjString *interned = table_find_string(&vm->strings, chars, length, hash);
  if (interned != NULL)
    return interned;
  char *value = allocate(sizeof(char), length + 1);
  memcpy(value, chars, length);
  value[length] = '\0';
  return allocate_string(vm, value, length, hash);
}

ObjString *allocate_string(VM *vm, char *chars, size_t length, uint32_t hash)
{
  ObjString *string = (ObjString *)allocate_object(vm, sizeof(ObjString), OBJ_STRING);
  string->chars = chars;
  string->length = length;
  string->hash = hash;
  table_set(&vm->strings, string, nil_val());
  return string;
}

Value concatenate(VM *vm, ObjString *sa, ObjString *sb)
{
  int length = sa->length + sb->length;
  char *chars = allocate(sizeof(char), length);

  memcpy(chars, sa->chars, sa->length);
  memcpy(chars + sa->length, sb->chars, sb->length);
  chars[length] = '\0';

  ObjString *sobj = take_string(vm, chars, length);
  return object_val((Obj *)sobj);
}

ObjString *take_string(VM *vm, char *chars, int length)
{
  uint32_t hash = hash_string(chars, length);
  ObjString *interned = table_find_string(&vm->strings, chars, length, hash);
  if (interned != NULL)
  {
    free_array(sizeof(char), chars, length);
    return interned;
  }

  return allocate_string(vm, chars, length, hash);
}

void print_object(Value value)
{
  switch (object_type(value))
  {
  case OBJ_STRING:
    fprintf(stderr, "%s", as_cstring(value));
    break;

  default:
    break;
  }
}

bool is_equal_object(Obj *obja, Obj *objb)
{
  switch (obja->type)
  {
  case OBJ_STRING:
    return obja == objb;

  default:
    return false;
  }
}

bool is_object_type(Value value, ObjType type)
{
  return object_type(value) == type;
}

bool is_string(Value value)
{
  return is_object(value) && is_object_type(value, OBJ_STRING);
}

ObjString *as_string(Value value)
{
  return (ObjString *)as_object(value);
}

char *as_cstring(Value value)
{
  return ((ObjString *)as_object(value))->chars;
}

uint32_t hash_string(const char *key, int length)
{
  uint32_t hash = 2166136261u;

  for (int i = 0; i < length; i++)
  {
    hash ^= key[i];
    hash *= 16777619;
  }

  return hash;
}
