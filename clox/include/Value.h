#ifndef _VALUE_H_
#define _VALUE_H_

#include <stddef.h>

typedef double Value;

struct ValueArray {
  size_t size;
  size_t capacity;
  Value *values;
};

typedef struct ValueArray ValueArray;

void init_value_array(ValueArray *array);
void write_value_array(ValueArray *array, Value value);
void free_value_array(ValueArray *array);

#endif
