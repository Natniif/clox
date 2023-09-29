#include <stdio.h>

#include "debug.h"
#include "value.h"

// disassemble the instructions until you get to the end of the chunk
void disassembleChunk(Chunk* chunk, const char* name) {
    printf("== %s ==\n", name);

    for (int offset = 0; offset < chunk->count;) {
        offset = disassembleInstruction(chunk, offset);
    }
}

static int simpleInstruction(const char* name, int offset) {
    printf("%s\n", name);
    return offset + 1;
}

// prints out opcode as well as the offset
int disassembleInstruction(Chunk* chunk, int offset) {
    printf("%04d ", offset);

    // if same line as before instruction just print |
    if (offset > 0 && 
        chunk->lines[offset] == chunk->lines[offset -1]) {
            printf("  | ");
        } else {
            printf("%4d ", chunk->lines[offset]);
        }

    uint8_t instruction = chunk->code[offset];
    switch (instruction)
    {
    case OP_CONSTANT:  
        return constantInstruction("OP_CONSTANT", chunk, offset);
    case OP_ADD:
        return simpleInstruction("OP_ADD", offset);
    case OP_SUBTRACT:
        return simpleInstruction("OP_SUBTRACT", offset);
    case OP_MULTIPLY:
        return simpleInstruction("OP_MULTIPLY", offset);
    case OP_DIVIDE:
        return simpleInstruction("OP_DIVIDE", offset);
    case OP_NEGATE:
        return simpleInstruction("OP_NEGATE", offset);
    case OP_RETURN:
        return simpleInstruction("OP_RETURN", offset);
    
    default:
        printf("Unknown opcode %d\n", instruction);
        return offset + 1;
    }
}


/*
As with OP_RETURN, we print out the name of the opcode
Then we pull out the constant index from the subsequent byte in the chunk
We print that index and look up the value and display the value itself too
*/
static int constantInstruction(const char* name, Chunk* chunk, int offset) {
    // constant value is offset of one from the opcode
    uint8_t constant = chunk->code[offset + 1];
    printf("%-16s %4d '", name, constant);
    printValue(chunk->constants.values[constant]);
    printf("'\n");
    // offset by two since the OP_CONSTANT has two bytes
        // - one for the opcode and one for the operand
    return offset + 2;
}