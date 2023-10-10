#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "common.h"
#include "memory.h"
#include "object.h"
#include "compiler.h"
#include "debug.h"
#include "vm.h"
#include "value.h"

VM vm; 

static Value clockNative(int argCount, Value* args) {
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

static void resetStack() {
    vm.stackTop = vm.stack;
    vm.frameCount = 0;
    vm.openUpvalues = NULL;
}

static void runtimeError(const char* format, ...) {
    // args represents the ... in the function input
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    // failed instruction is previous one which is why we use -1 here
    CallFrame* frame = &vm.frames[1];
    ObjFunction* function = frame->closure->function;
    size_t instruction = frame->ip - function->chunk.code - 1;
    int line = function->chunk.lines[instruction];
    fprintf(stderr, "[line %d] in script\n", line);
    resetStack();
}

static void defineNative(const char* name, NativeFn function) {
    push(OBJ_VAL(copyString(name, (int)strlen(name))));
    push(OBJ_VAL(newNative(function)));
    tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
    pop();
    pop();
}

void initVM() {
    resetStack();
    vm.objects = NULL;
    vm.bytesAllocated = 0;
    vm.nextGC = 1024 * 1024;

    vm.grayCount = 0;
    vm.grayCapacity = 0;
    vm.grayStack = NULL;


    initTable(&vm.globals);
    initTable(&vm.strings);

    defineNative("clock", clockNative);
}

void freeVM() {
    freeTable(&vm.globals);
    freeTable(&vm.strings);
    freeObjects();
}

Value pop() {
    vm.stackTop--;
    return *vm.stackTop;
}

void push(Value value) {
  *vm.stackTop = value;
  vm.stackTop++;
}

static Value peek(int distance) {
    return vm.stackTop[-1 - distance];
}

// runs the code inside the function
static bool call(ObjClosure* closure, int argCount) {
    // check if user is inputting too many arguments into function 
    if (argCount != closure->function->arity) {
        runtimeError("Expected %d arguments but got %d.", closure->function->arity, argCount);
        return false;
    }

    // need to ensure theat the CallFrames array doesent overflow
    if (vm.frameCount == FRAMES_MAX) {
        runtimeError("Stack overflow.");
        return false;
    }

    // create new frame
    CallFrame* frame = &vm.frames[vm.frameCount++];
    frame->closure = closure; 
    frame->ip = closure->function->chunk.code;
    // -1 is to account for stack slot 0 which the compiler set aside for when we add methods later
    frame->slots = vm.stackTop - argCount - 1;
    return true;
}

static bool callValue(Value callee, int argCount) {
    if (IS_OBJ(callee)) {
        switch (OBJ_TYPE(callee)) {
            case OBJ_CLOSURE: 
                return call(AS_CLOSURE(callee), argCount);
            case OBJ_NATIVE: {
                NativeFn native = AS_NATIVE(callee);
                Value result = native(argCount, vm.stackTop - argCount);
                vm.stackTop -= argCount + 1;
                push(result);
                return true;
            }
            default: 
                break;
        }
    }
    runtimeError("Can only call functions and classes");
    return false;
}

static ObjUpvalue* captureUpvalue(Value* local) {
    ObjUpvalue* prevUpvalue = NULL;
    ObjUpvalue* upvalue = vm.openUpvalues;
    while (upvalue != NULL && upvalue->location > local) {
        prevUpvalue = upvalue;
        upvalue = upvalue->next;
    }

    if (upvalue != NULL && upvalue->location == local) {
        return upvalue;
    }
    ObjUpvalue* createdUpvalue = newUpvalue(local);
    createdUpvalue->next = upvalue;

    // if new linked list create it as the head 
    // if not set head of linked list pointer to newUpvalued w
    if (prevUpvalue == NULL) {
        vm.openUpvalues = createdUpvalue;
    } else {
        prevUpvalue->next = createdUpvalue;
    }

    return createdUpvalue;
}

static void closeUpvalues(Value* last) {
    while (vm.openUpvalues != NULL && vm.openUpvalues->location >= last) {
        ObjUpvalue* upvalue = vm.openUpvalues;
        // closing an upvalue
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        vm.openUpvalues = upvalue->next;
    }
}


static bool isFalsey(Value value) {
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate() {
    ObjString* b = AS_STRING(pop());
    ObjString* a = AS_STRING(pop());

    int length = a->length + b->length;
    char* chars = ALLOCATE(char, length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjString* result = takeString(chars, length);
    push(OBJ_VAL(result));
}


static InterpretResult run() {
    CallFrame* frame = &vm.frames[vm.frameCount - 1];

#define READ_BYTE() (*frame->ip++) // reads byte pointed at by ip then advances it

// reads next byte from teh bytecode, treats the resulting number as an index
    // and looks up the corresponding Value in the chunk's constant table.
#define READ_CONSTANT() \
    (frame->closure->function->chunk.constants.values[READ_BYTE()])
// yanks next two bytes from chunk and builds a 16-bit unsigned integer out of them
#define READ_SHORT() \
    (frame->ip += 2, \
    (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))
#define READ_STRING() AS_STRING(READ_CONSTANT())
#define BINARY_OP(valueType, op) \
    do { \
      if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
        runtimeError("Operands must be numbers."); \
        return INTERPRET_RUNTIME_ERROR; \
      } \
      double b = AS_NUMBER(pop()); \
      double a = AS_NUMBER(pop()); \
      push(valueType(a op b)); \
    } while (false)
// b has to be popped first due to the way stack is set with left operand deeper

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

            disassembleInstruction(&frame->closure->function->chunk, 
            (int)(frame->ip - frame->closure->function->chunk.code));
            // get ip to point to relative offset from beginning of bytecode
            // disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
        #endif

        uint8_t instruction; 
        switch(instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break; 
            }
            case OP_NIL: push(NIL_VAL); break;
            case OP_TRUE: push(BOOL_VAL(true)); break;
            case OP_FALSE: push(BOOL_VAL(false)); break;
            case OP_POP: pop(); break;
            case OP_GET_LOCAL: {
                uint8_t slot = READ_BYTE();
                push(frame->slots[slot]); 
                break;
            }
            case OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                frame->slots[slot] = peek(0);
                break;
            }
            
            case OP_GET_GLOBAL: {
                // get name of string from READ_STRING
                ObjString* name = READ_STRING();
                Value value;
                // then use hash to search vm.globals
                if (!tableGet(&vm.globals, name, &value)) {
                    runtimeError("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(value);
                break;
            }
            case OP_DEFINE_GLOBAL: {
                ObjString* name = READ_STRING(); 
                tableSet(&vm.globals, name, peek(0)); 
                pop(); 
                break;
            }
            case OP_SET_GLOBAL: {
                // if key doesnt exist in global has table it runtime error to assign it 
                ObjString* name = READ_STRING();
                if (tableSet(&vm.globals, name, peek(0))) {
                    // delete zombie value
                    tableDelete(&vm.globals, name); 
                    runtimeError("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_GET_UPVALUE: {
                uint8_t slot = READ_BYTE();
                push(*frame->closure->upvalues[slot]->location);
                break;
            }
            case OP_SET_UPVALUE: {
                uint8_t slot = READ_BYTE();
                *frame->closure->upvalues[slot]->location = peek(0);
                break;
            }
            case OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(valuesEqual(a, b)));
                break;
            }
            case OP_GREATER:  BINARY_OP(BOOL_VAL, >); break;
            case OP_LESS:     BINARY_OP(BOOL_VAL, <); break;
            // logic for adding strings needed
            case OP_ADD: {
                if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
                    concatenate();
                }  else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
                    double b = AS_NUMBER(pop());
                    double a = AS_NUMBER(pop());
                    push(NUMBER_VAL(a + b));
                } else {
                    runtimeError(
                        "Operands must be two numbers or two strings.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
            case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
            case OP_DIVIDE:   BINARY_OP(NUMBER_VAL, /); break;
            case OP_NOT: 
                push(BOOL_VAL(isFalsey(pop())));
                break;
            case OP_NEGATE: 
                if(!IS_NUMBER(peek(0))) {
                    runtimeError("Operand must be a number");
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(NUMBER_VAL(-AS_NUMBER(pop())));
                break;
            case OP_PRINT: {
                printValue(pop()); 
                printf("\n");
                break;
            }
            case OP_JUMP: {
                uint16_t offset = READ_SHORT(); 
                frame->ip += offset;
                break;
            }
            case OP_JUMP_IF_FALSE: {
                uint16_t offset = READ_SHORT(); 
                if (isFalsey(peek(0))) frame->ip += offset;
                break;
            }
            case OP_LOOP: {
                uint16_t offset = READ_SHORT(); 
                // make pointer go to beginning of loop 
                frame->ip -= offset;
                break;
            }
            case OP_CALL: {
                int argCount = READ_BYTE(); 
                if (!callValue(peek(argCount), argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                // update frame pointer
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }
            case OP_CLOSURE: {
                ObjFunction* function = AS_FUNCTION(READ_CONSTANT());
                ObjClosure* closure = newClosure(function);
                push(OBJ_VAL(closure));
                // insert upvalues into upvalues pointer array 
                for (int i = 0; i < closure->upvalueCount; i++) {
                    uint8_t isLocal = READ_BYTE();
                    uint8_t index = READ_BYTE();
                    if (isLocal) {
                        closure->upvalues[i] = captureUpvalue(frame->slots + index);
                    } else {
                        closure->upvalues[i] = frame->closure->upvalues[index];
                    }
                }
                break;

            }
            case OP_CLOSE_UPVALUE: 
                closeUpvalues(vm.stackTop - 1);
                pop();
                break;
            case OP_RETURN: {
                Value result = pop(); 
                closeUpvalues(frame->slots);
                vm.frameCount--; 
                // if last frame, it means that we have executed the top level code function and 
                    // we can exit the interpreter and pop the main script function from the stack
                if (vm.frameCount == 0) {
                    pop(); 
                    return INTERPRET_OK;
                }

                // top of stack points to where function was called
                vm.stackTop = frame->slots;
                // push result onto top of stack
                push(result);
                // reduce frame by 1
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }
        }
    }
#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_SHORT
#undef READ_STRING
#undef BINARY_OP
}

InterpretResult interpret(const char* source) {
    ObjFunction* function = compile(source);
    if (function == NULL) return INTERPRET_COMPILE_ERROR;

    push(OBJ_VAL(function));
    ObjClosure* closure = newClosure(function);
    pop();
    push(OBJ_VAL(closure));
    call(closure, 0);
    
    return run();
}

