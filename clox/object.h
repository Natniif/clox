#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "chunk.h"
#include "value.h"
#include "table.h"

#define IS_CLASS(value)         isObjType(value, OBJ_CLASS)
#define IS_CLOSURE(value)       isObjType(value, OBJ_CLOSURE)
#define IS_FUNCTION(value)      isObjType(value, OBJ_FUNCTION)
#define IS_INSTANCE(value)      isObjType(value, OBJ_INSTANCE)
#define IS_NATIVE(value)        isObjType(value, OBJ_NATIVE)
#define OBJ_TYPE(value)         (AS_OBJ(value)->type)
#define IS_STRING(value)        isObjType(value, OBJ_STRING)


// These following two macros take a Value that is expected to contain a pointer to a 
//valid ObjString on the heap. The first one returns the ObjString* pointer. 
#define AS_CLASS(value)        ((ObjClass*)AS_OBJ(value))
#define AS_CLOSURE(value)      ((ObjClosure*)AS_OBJ(value))
//The second one steps through that to return the character array itself
#define AS_FUNCTION(value)     ((ObjFunction*)AS_OBJ(value))
#define AS_INSTANCE(value)     ((ObjInstance*)AS_OBJ(value))
// extracts the c functin pointer from a value representing a native function
#define AS_NATIVE(value) \
    (((ObjNative*)AS_OBJ(value))->function)
#define AS_STRING(value)       ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)      (((ObjString*)AS_OBJ(value))->chars)

typedef enum {
    OBJ_CLASS, 
    OBJ_CLOSURE,
    OBJ_FUNCTION,
    OBJ_INSTANCE,
    OBJ_NATIVE,
    OBJ_STRING,
    OBJ_UPVALUE
} ObjType;

// already typin value.h
struct Obj {
    ObjType type;
    bool isMarked;
    struct Obj* next;
};

typedef struct {
    Obj obj; 
    int arity; // number of parameters the function expects
    int upvalueCount;
    Chunk chunk;
    ObjString* name;
} ObjFunction;

typedef Value (*NativeFn)(int argCount, Value* args);

typedef struct {
    Obj obj; 
    NativeFn function;
} ObjNative;

struct ObjString {
    Obj obj;
    int length; 
    char* chars;
    uint32_t hash;
};

typedef struct ObjUpvalue {
    Obj obj;
    Value* location;
    Value closed;
    // lined list of next ObjUpvalue
    struct ObjUpvalue* next;
} ObjUpvalue;

typedef struct {
    Obj obj; 
    ObjFunction* function;
    // objectClosure does not own the ObjUpvalue array objects themselves
    // but it owns teh array containing pointers to those upvalues
    ObjUpvalue** upvalues;
    int upvalueCount;
} ObjClosure;

typedef struct {
    Obj obj; 
    ObjString* name; 
    Table methods;
} ObjClass;

typedef struct {
    Obj obj;
    ObjClass* klass; 
    Table fields; 
} ObjInstance;

ObjClass* newClass(ObjString* name);

ObjClosure* newClosure(ObjFunction* function);

ObjFunction* newFunction();
ObjInstance* newInstance(ObjClass* klass);

ObjNative* newNative(NativeFn function);

ObjString* takeString(char* chars, int length);
ObjString* copyString(const char* chars, int length);
ObjUpvalue* newUpvalue(Value* slot);

// we put this outside of the macro for the following reason 
    // macros evaluate the expression for each insance it is called 
    // e.g. SQUARE(5+1) will calculate (5+1) for each time it is called 
        // as opposed to only evaluating it once and using 6 each time
static inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

void printObject(Value value);

#endif