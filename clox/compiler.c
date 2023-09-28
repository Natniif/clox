#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"
#include "chunk.h"

typedef struct {
    Token current; 
    Token previous;
    bool hadError;
    bool panicMode;
} Parser;

// lox precedence from highest to lowest
typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_OR,          // or
    PREC_AND,         // and
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -
    PREC_FACTOR,      // * /
    PREC_UNARY,       // ! -
    PREC_CALL,        // . ()
    PREC_PRIMARY
} Precedence;


Parser parser;

Chunk* compilingChunk;

static Chunk* currentChunk() {
    return compilingChunk;
}

static void errorAt(Token* token, const char* message) {
    if (parser.panicMode) return;
    parser.panicMode = true;
    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) {
      fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {
      // Nothing.
    } else {
      fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

static void error(const char* message) {
    errorAt(&parser.current, message);
}

static void errorAtCurrent(const char* message) {
    errorAt(&parser.current, message);
}

static void advance() {
    parser.previous = parser.current;

    for (;;) {
        parser.current = scanToken();
        if (parser.current.type != TOKEN_ERROR) break;

        errorAtCurrent(parser.current.start);
    }
}

// similar to advance but it also validates that the token has as expected type
static void consume(TokenType type, const char* message) {
    if (parser.current.type == type) {
        advance();
        return;
    }

    errorAtCurrent(message);
}

// append single byte to chunk
static void emitByte(uint8_t byte) {
    writeChunk(currentChunk(), byte, parser.previous.line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

static void emitReturn() {
    emitByte(OP_RETURN);
}

static void emitConstant(Value value) {
    emitBytes(OP_CONSTANT, makeConstant(value));
}

// add constant adds given value to end of chunk's constant table and returns the index
// this function also makes sure we dont have too many constants 
static uint8_t makeConstant(Value value) {
    int constant = addConstant(currentChunk(), value);
    if (constant > UINT8_MAX) {
        error("Too many constants in one chunk");
        return 0;
    }

    // cast to 8 bit int
    return (uint8_t)constant;
}

static void endCompiler() {
    emitReturn();
}

// dealing with parenthesis
static void grouping() {
    // there is no opcode for parenthesis so all we are doing here is just 
    // generating the bypecode with a higher precedence
    expression(); 
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void number() {
    double value = strtod(parser.previous.start, NULL);
    emitConstant(value);
}

static void unary() {
    TokenType operatorType = parser.previous.type;

    // compile operand
    parsePrecedence(PREC_UNARY);

    // emit the operator instruction
    switch (operatorType) {
        case TOKEN_MINUS: emitByte(OP_NEGATE); break;
        default: return;
    }

}

// starts at current token and parses any expression at the given precedence level or higher
static void parsePrecedence(Precedence precedence) {

}

static void expression() {
    parsePrecedence(PREC_ASSIGNMENT);
}

void compile(const char* source, Chunk* chunk) {
    initScanner(source);
    compilingChunk = chunk; // shouldnt this be a reference to chunk? 

    parser.hadError = false;
    parser.panicMode = false;

    advance(); 
    expression(); 
    // consumes tokens until end of file
    consume(TOKEN_EOF, "Expect end of expression");
    // end compilation after end of file found
    endCompiler();
    return !parser.hadError; 

}
