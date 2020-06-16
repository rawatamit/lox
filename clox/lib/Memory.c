#include "Memory.h"
#include "VM.h"
#include <stdlib.h>

size_t grow_capacity(size_t capacity)
{
  return (capacity < 128) ? 128 : (capacity * 2);
}

void *grow_array(void *array, size_t element_size, size_t old_capacity,
                 size_t new_capacity)
{
  return reallocate(array, element_size * old_capacity,
                    element_size * new_capacity);
}

void free_array(size_t element_size, void *array, size_t capacity)
{
  reallocate(array, capacity * element_size, 0);
}

void *reallocate(void *array, size_t old_size, size_t new_size)
{
  if (new_size == 0)
  {
    free(array);
    return NULL;
  }

  return realloc(array, new_size);
}

void *allocate(size_t element_size, size_t capacity)
{
  return reallocate(NULL, 0, element_size * capacity);
}

Obj *allocate_object(VM *vm, size_t object_size, ObjType type)
{
  Obj *obj = reallocate(NULL, 0, object_size);
  obj->type = type;
  obj->next = vm->objects;
  vm->objects = obj;
  return obj;
}
