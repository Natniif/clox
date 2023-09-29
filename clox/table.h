#ifndef clox_table_h
#define clox_table_h

#include "common.h"
#include "value.h"

typedef struct {
    ObjString* key;
    Value value;
} Entry;

typedef struct {
    int count; 
    int capacity; 
    Entry* entries;
} Table;

void initTable(Table* table);
void freeTable(table);
bool tableGet(Table* table, ObjString* key, value* value);
bool tableSet(Table* table, ObjString* key, Value value);
bool tableDelete(table* table, ObjString* key);
void tableAddAll(Table* from, Table* to);

#endif