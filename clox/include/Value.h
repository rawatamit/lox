#ifndef _VALUE_H_
#define _VALUE_H_

#include <stdbool.h>
#include <stddef.h>

enum ValueType {
  VAL_BOOL,
  VAL_NIL,
  VAL_NUMBER,
  VAL_OBJ,
};

typedef enum ValueType ValueType;

typedef struct Obj Obj;
typedef struct ObjString ObjString;

struct Value {
  ValueType type;
  union {
    bool boolean;
    double number;
    Obj *obj;
  } as;
};

typedef struct Value Value;

struct ValueArray {
  size_t size;
  size_t capacity;
  Value *values;
};

typedef struct ValueArray ValueArray;

void init_value_array(ValueArray *array);
void write_value_array(ValueArray *array, Value value);
void free_value_array(ValueArray *array);
void print_value(Value value);

Value add(Value, Value);
Value subtract(Value, Value);
Value multiply(Value, Value);
Value divide(Value, Value);
Value greater(Value, Value);
Value less(Value, Value);

Value bool_val(bool value);
Value nil_val();
Value number_val(double value);
Value object_val(Obj *value);

bool as_bool(Value value);
double as_number(Value value);
Obj *as_object(Value value);

bool is_bool(Value value);
bool is_nil(Value value);
bool is_number(Value value);
bool is_object(Value value);

bool is_falsey(Value value);
bool is_equal(Value a, Value b);

#endif
