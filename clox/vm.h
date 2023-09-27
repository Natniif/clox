#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"

typedef struct
{
    Chunk* chunk;

    // instruction pointer
    // points to the instruction about to be executed
    uint8_t* ip;
} VM;

// runtime report errors
typedef enum {
    INTERPRET_OK, 
    INTERPRET_COMPILE_ERROR, 
    INTERPRET_RUNTIME_ERROR, 
} InterpretResult;

InterpretResult interpret(Chunk* chunk);

void initVM();
void freeVM();

#endif