#include "VM.h"
#include "Memory.h"
#include <stdlib.h>

#ifdef DEBUG_LOG_GC
#include "Debug.h"
#include <stdio.h>
#endif

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
  mark_object(vm, (Obj *)vm->init_string);
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

  case OBJ_CLASS:
  {
    ObjClass *klass = (ObjClass *)object;
    mark_object(vm, (Obj *)klass->name);
    mark_table(vm, &klass->methods);
    break;
  }

  case OBJ_BOUND_METHOD:
  {
    ObjBoundMethod *bound_method = (ObjBoundMethod *)object;
    mark_value(vm, bound_method->receiver);
    mark_object(vm, (Obj *)bound_method->method);
    break;
  }

  case OBJ_INSTANCE:
  {
    ObjInstance *instance = (ObjInstance *)object;
    mark_object(vm, (Obj *)instance->klass);
    mark_table(vm, &instance->fields);
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
