#include "Memory.h"
#include "Compiler.h"
#include "VM.h"
#include "Value.h"
#include <stdlib.h>

#ifdef DEBUG_LOG_GC
#include "Debug.h"
#include <stdio.h>
#endif

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

void collect_garbage(VM *vm)
{
#ifdef DEBUG_LOG_GC
  fprintf(stderr, "-- gc begin\n");
  size_t bytes_allocated_before = vm->bytes_allocated;
#endif

  mark_roots(vm);
  trace_references(vm);
  table_remove_white(vm, &vm->strings);
  sweep(vm);

  vm->next_gc = vm->bytes_allocated * GC_HEAP_GROW_FACTOR;

#ifdef DEBUG_LOG_GC
  fprintf(stderr, "-- gc end\n");
  fprintf(stderr, "   collected %ld bytes (from %ld to %ld) next at %ld\n",
          bytes_allocated_before - vm->bytes_allocated,
          bytes_allocated_before, vm->bytes_allocated,
          vm->next_gc);
#endif
}

void mark_roots(VM *vm)
{
  for (Value *slot = vm->stack; slot < vm->stack_top; ++slot)
  {
    mark_value(vm, *slot);
  }

  for (int i = 0; i < vm->frame_count; ++i)
  {
    mark_object(vm, (Obj *)vm->frames[i].closure);
  }

  for (ObjUpvalue *upvalue = vm->open_upvalues; upvalue != NULL;
       upvalue = upvalue->next)
  {
    mark_object(vm, (Obj *)upvalue);
  }

  mark_table(vm, &vm->globals);
  mark_compiler_roots(vm->compiler);
}

void mark_table(VM *vm, Table *table)
{
  for (Entry *cur = table->front; cur != NULL; cur = cur->next)
  {
    mark_object(vm, (Obj *)cur->key);
    mark_value(vm, cur->value);
  }
}

void mark_array(VM *vm, ValueArray *array)
{
  for (int i = 0; i < (int)array->size; ++i)
  {
    mark_value(vm, array->values[i]);
  }
}

void mark_value(VM *vm, Value value)
{
  if (!is_object(value))
    return;
  mark_object(vm, as_object(value));
}

void mark_object(VM *vm, Obj *object)
{
  if (object != NULL && !object->is_marked)
  {
#ifdef DEBUG_LOG_GC
    fprintf(stderr, "%p mark ", (void *)object);
    print_value(stderr, object_val(object));
    fputc('\n', stderr);
#endif

    object->is_marked = true;

    if (vm->gray_capacity < vm->gray_size + 1)
    {
      vm->gray_capacity = grow_capacity(vm->gray_capacity);
      vm->gray_stack =
          realloc(vm->gray_stack, sizeof(Obj *) * vm->gray_capacity);
    }

    vm->gray_stack[vm->gray_size++] = object;
  }
}

void trace_references(VM *vm)
{
  while (vm->gray_size > 0)
  {
    Obj *object = vm->gray_stack[--vm->gray_size];
    blacken_object(vm, object);
  }
}

void blacken_object(VM *vm, Obj *object)
{
#ifdef DEBUG_LOG_GC
  fprintf(stderr, "%p blacken ", (void *)object);
  print_value(stderr, object_val(object));
  fputc('\n', stderr);
#endif

  switch (object->type)
  {
  case OBJ_NATIVE:
  case OBJ_STRING:
    break;

  case OBJ_UPVALUE:
    mark_value(vm, ((ObjUpvalue *)object)->closed);
    break;

  case OBJ_FUNCTION:
  {
    ObjFunction *fn = (ObjFunction *)object;
    mark_object(vm, (Obj *)fn->name);
    mark_array(vm, &fn->chunk.constants);
    break;
  }

  case OBJ_CLOSURE:
  {
    ObjClosure *closure = (ObjClosure *)object;
    mark_object(vm, (Obj *)closure->fn);
    for (int i = 0; i < closure->upvalue_count; ++i)
    {
      mark_object(vm, (Obj *)closure->upvalues[i]);
    }

    break;
  }

  default:
    break;
  }
}

void table_remove_white(VM *vm, Table *table)
{
  for (Entry *cur = table->front; cur != NULL; cur = cur->next)
  {
    if (cur->key != NULL && !cur->key->obj.is_marked)
    {
#ifdef DEBUG_LOG_GC
      fprintf(stderr, "%p table_remove_white ", (void *)cur->key);
      print_value(stderr, object_val((Obj *)cur->key));
      fputc('\n', stderr);
#endif
      table_delete(vm, table, cur->key);
    }
  }
}

void sweep(VM *vm)
{
  Obj *prev = NULL;
  Obj *object = vm->objects;

  while (object != NULL)
  {
    if (object->is_marked)
    {
      object->is_marked = false;
      prev = object;
      object = object->next;
    }
    else
    {
      Obj *reclaim = object;
      object = object->next;

      // first node
      if (prev == NULL)
      {
        vm->objects = object;
      }
      else
      {
        prev->next = object;
      }

#ifdef DEBUG_LOG_GC
      fprintf(stderr, "%p reclaim ", (void *)reclaim);
      print_value(stderr, object_val(reclaim));
      fputc('\n', stderr);
#endif
      free_object(vm, reclaim);
    }
  }
}
