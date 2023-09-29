#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "value.h"

#define OBJ_TYPE(value)         (AS_OBJ(value)->type)
#define IS_STRING(value)        isObjType(value, OBJ_STRING)


// These following two macros take a Value that is expected to contain a pointer to a 
//valid ObjString on the heap. The first one returns the ObjString* pointer. 

//The second one steps through that to return the character array itself
#define AS_STRING(value)       ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)      (((ObjString*)AS_OBJ(value))->chars)

typedef enum {
    OBJ_STRING,
} ObjType;

// already typedef'd in value.h
struct Obj {
    ObjType type;
    struct Obj* next;
};

struct ObjString {
    Obj obj;
    int length; 
    char* chars;
};


ObjString* takeString(char* chars, int length);
ObjString* copyString(const char* chars, int length);

// we put this outside of the macro for the following reason 
    // macros evaluate the expression for each insance it is called 
    // e.g. SQUARE(5+1) will calculate (5+1) for each time it is called 
        // as opposed to only evaluating it once and using 6 each time
static inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

void printObject(Value value);

#endif