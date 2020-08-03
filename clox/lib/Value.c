#include "Value.h"
#include "Memory.h"
#include <stdint.h>
#include <stdio.h>

void init_value_array(ValueArray *array)
{
  array->size = 0;
  array->capacity = 0;
  array->values = NULL;
}

void write_value_array(VM *vm, ValueArray *array, Value value)
{
  if (array->capacity <= array->size)
  {
    size_t old_capacity = array->capacity;
    array->capacity = grow_capacity(old_capacity);
    array->values =
        grow_array(vm, array->values, sizeof(Value), old_capacity, array->capacity);
  }

  array->values[array->size++] = value;
}

void free_value_array(VM *vm, ValueArray *array)
{
  reallocate(vm, array->values, array->capacity, 0);
  init_value_array(array);
}

void print_value(FILE *out, Value value)
{
  switch (value.type)
  {
  case VAL_BOOL:
    fprintf(out, as_bool(value) ? "true" : "false");
    break;
  case VAL_NIL:
    fprintf(out, "nil");
    break;
  case VAL_NUMBER:
    fprintf(out, "%g", as_number(value));
    break;
  case VAL_OBJ:
    print_object(out, value);
    break;
  default:
    break;
  }
}

Value add(Value a, Value b) { return number_val(as_number(a) + as_number(b)); }

Value subtract(Value a, Value b)
{
  return number_val(as_number(a) - as_number(b));
}

Value multiply(Value a, Value b)
{
  return number_val(as_number(a) * as_number(b));
}

Value divide(Value a, Value b)
{
  return number_val(as_number(a) / as_number(b));
}

Value greater(Value a, Value b)
{
  return bool_val(as_number(a) > as_number(b));
}

Value less(Value a, Value b) { return bool_val(as_number(a) < as_number(b)); }

Value bool_val(bool value)
{
  return (Value){.type = VAL_BOOL, {.boolean = value}};
}

Value nil_val() { return (Value){.type = VAL_NIL, {.number = 0}}; }

Value number_val(double value)
{
  return (Value){.type = VAL_NUMBER, {.number = value}};
}

Value object_val(Obj *value)
{
  return (Value){.type = VAL_OBJ, {.obj = value}};
}

bool as_bool(Value value) { return value.as.boolean; }

double as_number(Value value) { return value.as.number; }

Obj *as_object(Value value) { return value.as.obj; }

bool is_bool(Value value) { return value.type == VAL_BOOL; }

bool is_nil(Value value) { return value.type == VAL_NIL; }

bool is_number(Value value) { return value.type == VAL_NUMBER; }

bool is_object(Value value) { return value.type == VAL_OBJ; }

bool is_falsey(Value value)
{
  switch (value.type)
  {
  case VAL_BOOL:
    return !as_bool(value);

  case VAL_NIL:
    return true;

  default:
    return false;
  }
}

bool is_equal(Value a, Value b)
{
  if (a.type != b.type)
  {
    return false;
  }
  else
  {
    switch (a.type)
    {
    case VAL_BOOL:
      return as_bool(a) == as_bool(b);

    case VAL_NIL:
      return true;

    case VAL_NUMBER:
      return as_number(a) == as_number(b);

    case VAL_OBJ:
      return as_object(a) == as_object(b);

    default:
      return false;
    }
  }
}
