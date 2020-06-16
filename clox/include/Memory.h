#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "Object.h"
#include <stddef.h>

size_t grow_capacity(size_t capacity);
void *grow_array(void *array, size_t element_size, size_t old_capacity,
                 size_t new_capacity);
void free_array(size_t element_size, void *array, size_t capacity);
void *reallocate(void *array, size_t old_size, size_t new_size);
void *allocate(size_t element_size, size_t capacity);
Obj *allocate_object(VM *vm, size_t object_size, ObjType type);

#endif
