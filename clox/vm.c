#include <stdio.h>
#include "common.h"
#include "vm.h"

VM vm; 

void initVM() {

}

void freeVM() {

}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++) // reads byte pointed at by ip then advances it

// reads next byte from teh bytecode, treats the resulting number as an index
    // and looks up the corresponding Value in the chunk's constant table.
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

    for (;;) {


        uint8_t instruction; 
        switch(instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                printValue(constant);
                printf("\n");
                break; 
            }
            case OP_RETURN: {
                return INTERPRET_OK;
            }
        }
    }
#undef READ_BYTE
#undef READ_CONSTANT
}

InterpretResult interpret(Chunk* chunk) {
    vm.chunk = chunk;
    vm.ip = vm.chunk->code;

    // run the bytecode
    return run();
}