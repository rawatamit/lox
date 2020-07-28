#ifndef _TABLE_H_
#define _TABLE_H_

#include "Value.h"
#include <stdint.h>

typedef struct ObjString ObjString;

struct Entry {
  ObjString *key;
  Value value;
  struct Entry *next;
};

typedef struct Entry Entry;

struct Table {
  Entry *front;
  Entry *rear;
  int size;
  int capacity;
};

typedef struct Table Table;

void init_table(Table *table);
void free_table(VM *vm, Table *table);
bool table_set(VM *vm, Table *table, ObjString *key, Value value);
bool table_get(Table *table, ObjString *key, Value *value);
ObjString *table_find_string(Table *table, const char *key, size_t length,
                             uint32_t hash);
bool table_delete(VM *vm, Table *table, ObjString *key);
void table_add_all(Table *from, Table *to);

#endif
