#include "Table.h"
#include "Memory.h"
#include <string.h>

static Entry *find_entry(Entry *entries, ObjString *key);
static void add_element(Entry **front, Entry **rear, Entry *entry);

void init_table(Table *table) {
  table->size = 0;
  table->capacity = 0;
  table->front = NULL;
  table->rear = NULL;
}

void free_table(VM *vm, Table *table) {
  for (Entry *cur = table->front; cur != NULL;) {
    Entry *next = cur->next;
    free_array(vm, sizeof(Entry), cur, 1);
    cur = next;
  }

  init_table(table);
}

bool table_set(VM *vm, Table *table, ObjString *key, Value value) {
  Entry *entry = find_entry(table->front, key);

  if (entry == NULL) {
    return table_set_no_search(vm, table, key, value);
  } else {
    entry->value = value;
    return false;
  }
}

bool table_set_no_search(VM *vm, Table *table, ObjString *key, Value value) {
  Entry *entry = allocate(vm, sizeof(Entry), 1);
  entry->key = key;
  entry->value = value;
  add_element(&table->front, &table->rear, entry);
  ++table->size;
  ++table->capacity;
  return true;
}

bool table_get(Table *table, ObjString *key, Value *value) {
  if (table->size == 0)
    return false;

  Entry *entry = find_entry(table->front, key);
  if (entry == NULL)
    return false;

  *value = entry->value;
  return true;
}

ObjString *table_find_string(Table *table, const char *key, size_t length,
                             uint32_t hash) {
  for (Entry *cur = table->front; cur != NULL; cur = cur->next) {
    if (cur->key->hash == hash && cur->key->length == length &&
        memcmp(cur->key->chars, key, length) == 0) {
      return cur->key;
    }
  }

  return NULL;
}

bool table_delete(VM *vm, Table *table, ObjString *key) {
  if (table->size == 0)
    return false;

  Entry *prev = NULL;
  Entry *cur = table->front;

  while (cur != NULL && cur->key != key) {
    prev = cur;
    cur = cur->next;
  }

  // didn't find the entry
  if (cur == NULL) {
    return false;
  } else if (prev == NULL) // first entry
  {
    table->front = table->front->next;
    free_array(vm, sizeof(Entry), cur, 1);
    --table->size;
    return true;
  } else {
    prev->next = cur->next;
    free_array(vm, sizeof(Entry), cur, 1);
    --table->size;
    return true;
  }
}

void table_add_all(VM *vm, Table *from, Table *to) {
  for (Entry *cur = from->front; cur != NULL; cur = cur->next) {
    table_set_no_search(vm, to, cur->key, cur->value);
  }
}

Entry *find_entry(Entry *entries, ObjString *key) {
  for (Entry *cur = entries; cur != NULL; cur = cur->next) {
    if (cur->key == key) {
      return cur;
    }
  }

  return NULL;
}

void add_element(Entry **front, Entry **rear, Entry *entry) {
  if (*front == NULL) {
    *front = *rear = entry;
  } else {
    entry->next = *front;
    *front = entry;
  }
}
