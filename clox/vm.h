#ifndef clox_vm_h
#define clox_vm_h

#include "object.h"
#include "chunk.h"
#include "table.h"
#include "value.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

// represents a single ongoig function call
typedef struct {
    ObjClosure* closure; 
    uint8_t* ip; 
    Value* slots; 
} CallFrame;

typedef struct
{
    CallFrame frames[FRAMES_MAX];
    int frameCount;

    Value stack[STACK_MAX];
    
    // stackTop points to the value just above the 'freshest' value
    Value* stackTop;
    Table globals;
    Table strings;
    ObjUpvalue* openUpvalues;

    size_t bytesAllocated;
    size_t nextGC;

    Obj* objects;

    int grayCount;
    int grayCapacity;
    Obj** grayStack;
} VM;

// runtime report errors
typedef enum {
    INTERPRET_OK, 
    INTERPRET_COMPILE_ERROR, 
    INTERPRET_RUNTIME_ERROR, 
} InterpretResult;

extern VM vm;

void initVM();
void freeVM();

InterpretResult interpret(const char* source);
void push(Value value);
Value pop();



#endif