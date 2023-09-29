#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "value.h"

#define STACK_MAX 256

typedef struct
{
    Chunk* chunk;

    // instruction pointer
    // points to the instruction about to be executed
    uint8_t* ip;
    Value stack[STACK_MAX];
    
    // stackTop points to the value just above the 'freshest' value
    Value* stackTop;

    Obj* objects;
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