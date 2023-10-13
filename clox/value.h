#ifndef clox_value_h
#define clox_value_h

#include "common.h"
#include "value.h"
#include <string.h>

typedef struct Obj Obj;
typedef struct ObjString ObjString;

#ifdef NAN_BOXING

// quiet NaN value
#define QNAN     ((uint64_t)0x7ffc000000000000)
#define SIGN_BIT ((uint64_t)0x8000000000000000)

#define TAG_NIL   1 // 01.
#define TAG_FALSE 2 // 10.
#define TAG_TRUE  3 // 11.

typedef uint64_t Value;

#define IS_BOOL(value)      (((value) | 1) == TRUE_VAL)
#define IS_NIL(value)       ((value) == NIL_VAL)
// if all NaN bits set and all quiet NaN bits set it is a number
#define IS_NUMBER(value)    (((value) & QNAN) != QNAN)
#define IS_OBJ(value) \
    (((value) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))

#define AS_BOOL(value)      ((value) == TRUE_VAL)
#define AS_NUMBER(value)    valueToNum(value)

// ~ is a bitwise NOT 
// allows us to clear those bits and let pointer bits remain
#define AS_OBJ(value) \
    ((Obj*)(uintptr_t)((value) & ~(SIGN_BIT | QNAN)))


#define NUMBER_VAL(num) numToValue(num)
#define BOOL_VAL(b)     ((b) ? TRUE_VAL : FALSE_VAL)
#define FALSE_VAL       ((Value)(uint64_t)(QNAN | TAG_FALSE))
#define TRUE_VAL        ((Value)(uint64_t)(QNAN | TAG_TRUE))
#define NIL_VAL         ((Value)(uint64_t)(QNAN | TAG_NIL))

#define OBJ_VAL(obj) \
    (Value)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(obj))


/* TYPE PUNNING
copy the bits from one type to the new Value space
this is a supported idiom for type punnign so compilers 
    recognise this pattern and optimize away memcpy entirely
if compiler does not support use this: 

double valueToNum(Value value) {
    union {
        uint64_t bits;
        double num;
    } data;
    data.bits = value;
    return data.num;
}

*/

static inline double valueToNum(Value value) {
    double num;
    memcpy(&num, &value, sizeof(Value));
    return num;
}

static inline Value numToValue(double num) {
    Value value;
    memcpy(&value, &num, sizeof(double));
    return value;
}

#else

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

#endif

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