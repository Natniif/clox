#ifndef clox_value_h
#define clox_value_h

#include "common.h"
#include "value.h"

typedef struct Obj Obj;
typedef struct ObjString ObjString;

typedef enum {
    VAL_BOOL, 
    VAL_INT, 
    VAL_NIL,
    VAL_NUMBER,
    // every lox value who's state lives on the heap is an Obj
    VAL_OBJ
} ValueType;

// typedef double Value;
typedef struct {
    ValueType type; 
    // we want to have overlapping fields for the 8 bits to show if it is bool or int
    // we can achieve this with a union 
    union {
        bool boolean; 
        double number; 
        // pointer since we need dynamic memory
        Obj* obj;
    } as;
} Value;


// these macros are dangerous if used wrong though e.g. double number = AS_NUMBER(value);
// so we add following macro the check the appropriate type
#define IS_BOOL(value)    ((value).type == VAL_BOOL)
#define IS_NIL(value)     ((value).type == VAL_NIL)
#define IS_NUMBER(value)  ((value).type == VAL_NUMBER)
#define IS_OBJ(value)     ((value).type == VAL_OBJ)

// These macros typecast a value of either boolean or number to the Value struct
// where the bits are assigned
#define AS_BOOL(value)   ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)
#define AS_OBJ(value)    ((value).as.obj) 

// translating native C value to clox Value struct
#define BOOL_VAL(value)   ((Value){VAL_BOOL, {.boolean = value}})
#define NIL_VAL           ((Value){VAL_NIL, {.number = 0}})
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})
#define OBJ_VAL(object)   ((Value){VAL_OBJ, {.obj = (Obj*)object}})


typedef struct {
    int capacity; 
    int count; 
    Value* values; 
} ValueArray;

bool valuesEqual(Value a, Value b);

void initValueArray(ValueArray* array);
void writeValueArray(ValueArray* array, Value value);
void freeValueArray(ValueArray* array);

void printValue(Value value);

#endif  