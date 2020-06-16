#include "Value.h"
#include "Memory.h"
#include <stdint.h>

void init_value_array(ValueArray *array) {
  array->size = 0;
  array->capacity = 0;
  array->values = NULL;
}

void write_value_array(ValueArray *array, Value value) {
  if (array->capacity <= array->size) {
    size_t old_capacity = array->capacity;
    array->capacity = grow_capacity(old_capacity);
    array->values =
        grow_array(array->values, sizeof(Value), old_capacity, array->capacity);
  }

  array->values[array->size++] = value;
}

void free_value_array(ValueArray *array) {
  reallocate(array->values, array->capacity, 0);
  init_value_array(array);
}
