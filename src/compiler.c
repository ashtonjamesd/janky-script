#include <stdlib.h>
#include <stdio.h>

#include "compiler.h"

void initCompiler(Compiler *compiler, Ast *ast) {
    compiler->ast = ast;
    compiler->bytecode = malloc(sizeof(Bytecode));
    
    compiler->bytecode->code = malloc(sizeof(OpCode));
    compiler->bytecode->code_capacity = 1;
    compiler->bytecode->code_count = 0;

    compiler->bytecode->constants = malloc(sizeof(int));
    compiler->bytecode->const_capacity = 1;
    compiler->bytecode->const_count = 0;
}

static void emitByte(Bytecode *bytecode, OpCode op) {
    if (bytecode->code_count >= bytecode->code_capacity) {
        bytecode->code_capacity *= 2;
        bytecode->code = realloc(bytecode->code, sizeof(OpCode) * bytecode->code_capacity);
    }

    bytecode->code[bytecode->code_count++] = op;
}

static int addConstant(Bytecode *bytecode, Value value) {
    if (bytecode->const_count >= bytecode->const_capacity) {
        bytecode->const_capacity *= 2;
        bytecode->constants = realloc(bytecode->constants, sizeof(Value) * bytecode->const_capacity);
    }
    bytecode->constants[bytecode->const_count] = value;

    return bytecode->const_count++;
}

static void emitOperator(Bytecode *bytecode, TokenType op) {
    switch (op) {
        case PLUS: {
            emitByte(bytecode, OP_PLUS);
            break;
        }
        case MINUS: {
            emitByte(bytecode, OP_MINUS);
            break;
        }
        case STAR: {
            emitByte(bytecode, OP_MULTIPLY);
            break;
        }
        case SLASH: {
            emitByte(bytecode, OP_DIVIDE);
            break;
        }
        case MODULO: {
            emitByte(bytecode, OP_MODULO);
            break;
        }
        case LOGICAL_AND: {
            emitByte(bytecode, OP_LOGICAL_AND);
            break;
        }
        case LOGICAL_OR: {
            emitByte(bytecode, OP_LOGICAL_OR);
            break;
        }
        case BITWISE_AND: {
            emitByte(bytecode, OP_BITWISE_AND);
            break;
        }
        case BITWISE_OR: {
            emitByte(bytecode, OP_BITWISE_OR);
            break;
        }
        case BITWISE_XOR: {
            emitByte(bytecode, OP_BITWISE_XOR);
            break;
        }
        case BITWISE_NOT: {
            emitByte(bytecode, OP_BITWISE_NOT);
            break;
        }
        case DOUBLE_EQUALS: {
            emitByte(bytecode, OP_EQUALS);
            break;
        }
        case NOT_EQUALS: {
            emitByte(bytecode, OP_NOT_EQUALS);
            break;
        }
        case GREATER_THAN: {
            emitByte(bytecode, OP_GREATER_THAN);
            break;
        }
        case LESS_THAN: {
            emitByte(bytecode, OP_LESS_THAN);
            break;
        }
        case GREATER_THAN_EQUALS: {
            emitByte(bytecode, OP_GREATER_THAN_EQUALS);
            break;
        }
        case LESS_THAN_EQUALS: {
            emitByte(bytecode, OP_LESS_THAN_EQUALS);
            break;
        }
        case BITWISE_LEFT_SHIFT: {
            emitByte(bytecode, OP_BITWISE_LEFT_SHIFT);
            break;
        }
        case BITWISE_RIGHT_SHIFT: {
            emitByte(bytecode, OP_BITWISE_RIGHT_SHIFT);
            break;
        }
        case TRIPLE_EQUALS: {
            emitByte(bytecode, OP_TRIPLE_EQUALS);
            break;
        }
        case TRIPLE_NOT_EQUALS: {
            emitByte(bytecode, OP_TRIPLE_NOT_EQUALS);
            break;
        }
        default: {
            fprintf(stderr, "Unknown operator in compiler, stopping.\n");
            exit(EXIT_FAILURE);
        }
    }
}

static void compileExpr(Bytecode *bytecode, AstExpression *expr) {
    switch (expr->type) {
        case AST_CONSTANT: {
            Value val;
            val.type = expr->as.constant.type;
            if (val.type == TYPE_NUMBER) {
                val.as.number = expr->as.constant.as.number;
            } else if (val.type == TYPE_BOOL) {
                val.as.boolean = expr->as.constant.as.boolean;
            } else if (val.type == TYPE_STRING) {
                val.as.object = expr->as.constant.as.object;
            } else {
                fprintf(stderr, "Unknown constant type in compiler.\n");
                exit(EXIT_FAILURE);
            }

            int index = addConstant(bytecode, val);
            emitByte(bytecode, OP_CONSTANT);
            emitByte(bytecode, index);
            break;
        }
        case AST_UNARY: {
            compileExpr(bytecode, expr->as.unary.right);
            if (expr->as.unary.op == MINUS) {
                emitByte(bytecode, OP_NEGATE);
            } else if (expr->as.unary.op == LOGICAL_NOT) {
                emitByte(bytecode, OP_LOGICAL_NOT);
            } else if (expr->as.unary.op == BITWISE_NOT) {
                emitByte(bytecode, OP_BITWISE_NOT);
            } else {
                fprintf(stderr, "Unknown unary op");
                exit(EXIT_FAILURE);
            }
            break;
        }
        case AST_BINARY: {
            compileExpr(bytecode, expr->as.binary.right);
            compileExpr(bytecode, expr->as.binary.left);
            emitOperator(bytecode, expr->as.binary.op);
            break;
        }
        default: {
            printf("Unable to compile expression as it is unknown.\n");
            exit(EXIT_FAILURE);
        }
    }
}

void compile(Compiler *compiler) {
    for (int i = 0; i < compiler->ast->expr_count; i++) {
        compileExpr(compiler->bytecode, compiler->ast->exprs[i]);
    }

    emitByte(compiler->bytecode, OP_END);
}