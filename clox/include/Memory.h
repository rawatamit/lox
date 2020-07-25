#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "Object.h"
#include "Table.h"
#include <stddef.h>

#define GC_HEAP_GROW_FACTOR 2

typedef struct Value Value;
typedef struct VM VM;
typedef struct Compiler Compiler;

size_t grow_capacity(size_t capacity);
void *grow_array(VM *vm, void *array, size_t element_size, size_t old_capacity,
                 size_t new_capacity);
void free_array(VM *vm, size_t element_size, void *array, size_t capacity);
void *reallocate(VM *vm, void *array, size_t old_size, size_t new_size);
void *allocate(VM *vm, size_t element_size, size_t capacity);
Obj *allocate_object(VM *vm, size_t object_size, ObjType type);
void free_object(VM *vm, Obj *obj);

void collect_garbage(VM *vm);
void mark_roots(VM *vm);
void mark_compiler_roots(Compiler *compiler);
void mark_table(VM *vm, Table *table);
void mark_array(VM *vm, ValueArray *array);
void mark_value(VM *vm, Value value);
void mark_object(VM *vm, Obj *object);
void trace_references(VM *vm);
void blacken_object(VM *vm, Obj *object);
void table_remove_white(VM *vm, Table *table);
void sweep(VM *vm);

#endif
