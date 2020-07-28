#include "Memory.h"
#include "Compiler.h"
#include "VM.h"
#include "Value.h"
#include <stdlib.h>

size_t grow_capacity(size_t capacity)
{
  return (capacity < 128) ? 128 : (capacity * 2);
}

void *grow_array(VM *vm, void *array, size_t element_size, size_t old_capacity,
                 size_t new_capacity)
{
  return reallocate(vm, array, element_size * old_capacity,
                    element_size * new_capacity);
}

void free_object(VM *vm, Obj *obj)
{
#ifdef DEBUG_LOG_GC
  fprintf(stderr, "%p free type %d\n", (void *)obj, obj->type);
#endif

  switch (obj->type)
  {
  case OBJ_CLASS:
  {
    ObjClass *klass = (ObjClass *)obj;
    free_table(vm, &klass->methods);
    reallocate(vm, obj, sizeof(ObjClass), 0);
    break;
  }

  case OBJ_BOUND_METHOD:
  {
    reallocate(vm, obj, sizeof(ObjBoundMethod), 0);
    break;
  }

  case OBJ_INSTANCE:
  {
    ObjInstance *instance = (ObjInstance *)obj;
    free_table(vm, &instance->fields);
    reallocate(vm, obj, sizeof(ObjFunction), 0);
    break;
  }

  case OBJ_FUNCTION:
  {
    ObjFunction *fn = (ObjFunction *)obj;
    free_chunk(vm, &fn->chunk);
    reallocate(vm, obj, sizeof(ObjFunction), 0);
    break;
  }

  case OBJ_NATIVE:
  {
    reallocate(vm, obj, sizeof(ObjNative), 0);
    break;
  }

  case OBJ_CLOSURE:
  {
    ObjClosure *closure = (ObjClosure *)obj;
    free_array(vm, sizeof(ObjUpvalue *), closure->upvalues,
               closure->upvalue_count);
    reallocate(vm, obj, sizeof(ObjClosure), 0);
    break;
  }

  case OBJ_UPVALUE:
  {
    reallocate(vm, obj, sizeof(ObjUpvalue), 0);
    break;
  }

  case OBJ_STRING:
  {
    ObjString *string = (ObjString *)obj;
    // free string
    free_array(vm, sizeof(char), string->chars, string->length + 1);
    // free object
    reallocate(vm, obj, sizeof(ObjString), 0);
    break;
  }

  default:
    break;
  }
}

void free_array(VM *vm, size_t element_size, void *array, size_t capacity)
{
  reallocate(vm, array, capacity * element_size, 0);
}

void *reallocate(VM *vm, void *array, size_t old_size, size_t new_size)
{
  vm->bytes_allocated += new_size - old_size;

  if (new_size > old_size)
  {
#ifdef DEBUG_STRESS_GC
    collect_garbage(vm);
#endif
  }

  if (vm->bytes_allocated > vm->next_gc)
  {
    collect_garbage(vm);
  }

  if (new_size == 0)
  {
    free(array);
    return NULL;
  }

  return realloc(array, new_size);
}

void *allocate(VM *vm, size_t element_size, size_t capacity)
{
  return reallocate(vm, NULL, 0, element_size * capacity);
}

Obj *allocate_object(VM *vm, size_t object_size, ObjType type)
{
  Obj *obj = reallocate(vm, NULL, 0, object_size);
  obj->type = type;
  obj->is_marked = false;
  obj->next = vm->objects;
  vm->objects = obj;

#ifdef DEBUG_LOG_GC
  fprintf(stderr, "%p allocate %ld for %d\n", (void *)obj, object_size, type);
#endif

  return obj;
}
