#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"

typedef enum {
    OP_CONSTANT,
    OP_NEGATE,
    OP_RETURN,
} OpCode;

// chunk of data represents all the data that is sent to the CPU as instructions
// needs to be dynamic since we dont know how big the instruction has to be
// When we add an element, if the count is less than the capacity, 
    // then there is already available space in the array. 
    // We store the new element right in there and bump the count.
// if not then we copy the array with more size and find a new arena for it to be copied to 
typedef struct {
    int count; 
    int capacity;
    int* lines;
    ValueArray constants;
    uint8_t* code; 
} Chunk;

void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
int addConstant(Chunk* chunk, Value value);

#endif