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
    OP_NOT,
    OP_PLUS,
    OP_MINUS,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_MODULO,
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