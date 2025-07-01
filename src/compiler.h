#ifndef compiler_h
#define compiler_h

#include "parser.h"

typedef enum {
  VM_OK,
  VM_COMPILE_ERROR,
  VM_RUNTIME_ERROR,
} VmResult;

typedef enum {
    OP_CONSTANT,

    OP_NEGATE,
    
    OP_PLUS,
    OP_MINUS,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_MODULO,

    OP_LOGICAL_AND,
    OP_LOGICAL_OR,
    OP_LOGICAL_NOT,

    OP_EQUALS,
    OP_NOT_EQUALS,
    OP_LESS_THAN,
    OP_GREATER_THAN,
    OP_LESS_THAN_EQUALS,
    OP_GREATER_THAN_EQUALS,

    OP_BITWISE_AND,
    OP_BITWISE_OR,
    OP_BITWISE_NOT,
    OP_BITWISE_XOR,
    OP_BITWISE_LEFT_SHIFT,
    OP_BITWISE_RIGHT_SHIFT,

    OP_TRIPLE_EQUALS,
    OP_TRIPLE_NOT_EQUALS,

    OP_END,
} OpCode;

typedef struct {
    OpCode *code;
    int     code_capacity;
    int     code_count;
    
    Value  *constants;
    int     const_capacity;
    int     const_count;
} Bytecode;

typedef struct {
    Ast *ast;
    Bytecode *bytecode;
} Compiler;

void initCompiler(Compiler *compiler, Ast *ast);
void compile(Compiler *compiler);

#endif