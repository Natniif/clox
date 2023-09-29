#include <stdio.h>
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "vm.h"

VM vm; 

static void resetStack() {
    vm.stackTop = vm.stack;
}

void initVM() {
    resetStack();
}

void freeVM() {

}

void push(Value value) {
    // points the stackTop to value 
    *vm.stackTop = value;
    // increments stackTop pointer by one since it needs to point one past the top
    vm.stackTop++;
}

Value pop() {
    // move stack pointer down one value
    vm.stackTop--;
    // return -1 stackTop pointer
    return *vm.stackTop;
}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++) // reads byte pointed at by ip then advances it

// reads next byte from teh bytecode, treats the resulting number as an index
    // and looks up the corresponding Value in the chunk's constant table.
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

// b has to be popped first due to the way stack is set with left operand deeper
#define BINARY_OP(op) \
    do { \
        double b = pop(); \
        double a = pop(); \
        push(a op b); \
    } while (false)

    for (;;) {

        // debug options
        #ifdef DEBUG_TRACE_EXECUTION 
            printf("        ");
            // loops over stack and prints out each value, ending when reach top
            for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
                printf("[ ");
                printValue(*slot);
                printf(" ]");
            }
            printf("\n");

            disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
        #endif

        uint8_t instruction; 
        switch(instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break; 
            }
            case OP_ADD:      BINARY_OP(+); break;
            case OP_SUBTRACT: BINARY_OP(-); break;
            case OP_MULTIPLY: BINARY_OP(*); break;
            case OP_DIVIDE:   BINARY_OP(/); break;
            // pushes negative of popped stack onto top of stack again
            case OP_NEGATE: push(-pop()); break;
            case OP_RETURN: {
                printValue(pop());
                printf("\n");
                return INTERPRET_OK;
            }
        }
    }
#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

InterpretResult interpret(Chunk* chunk) {
    vm.chunk = chunk;
    vm.ip = vm.chunk->code;

    // run the bytecode
    return run();
}