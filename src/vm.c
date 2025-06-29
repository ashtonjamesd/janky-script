#include <stdio.h>
#include <stdlib.h>

#include "vm.h"
#include "compiler.h"

void initVm(JankyVm *vm, Bytecode *bytecode) {
    vm->bytecode = bytecode;
    vm->ip = 0;
    vm->stack_top = 0;
}

void freeVm(JankyVm *vm) {
    free(vm->bytecode->code);
    free(vm->bytecode->constants);
    free(vm->bytecode);
}

void push(JankyVm *vm, int value) {
    if (vm->stack_top >= STACK_MAX) {
        fprintf(stderr, "Stack overflow\n");
        exit(1);
    }
    vm->stack[vm->stack_top++] = value;
}

int pop(JankyVm *vm) {
    if (vm->stack_top == 0) {
        fprintf(stderr, "Stack underflow\n");
        exit(1);
    }
    return vm->stack[--vm->stack_top];
}

static void evalOpCode(JankyVm *vm, OpCode op) {
    switch (op) {
        case OP_CONSTANT: {
            OpCode constIdx = vm->bytecode->code[vm->ip];
            vm->ip++;
            
            push(vm, vm->bytecode->constants[constIdx]);

            break;
        }
        case OP_PLUS: {
            int a = pop(vm);
            int b = pop(vm);

            push(vm, a + b);
            break;
        }
        case OP_MINUS: {
            int a = pop(vm);
            int b = pop(vm);

            push(vm, a - b);
            break;
        }
        case OP_MULTIPLY: {
            int a = pop(vm);
            int b = pop(vm);

            push(vm, a * b);
            break;
        }
        case OP_DIVIDE: {
            int a = pop(vm);
            int b = pop(vm);

            push(vm, a / b);
            break;
        }
        case OP_MODULO: {
            int a = pop(vm);
            int b = pop(vm);

            push(vm, a % b);
            break;
        }
        case OP_NEGATE: {
            push(vm, -pop(vm));
            break;
        }
        case OP_END: {
            if (vm->stack_top == 0) {
                return;
            }

            printf("%d\n", pop(vm));
            break;
        }
        default: {
            fprintf(stderr, "Halting execution: unknown opcode '%d'\n", op);
            exit(1);
        }
    }
}

VmResult run(JankyVm *vm, char *source, int debug) {
    Lexer lexer;
    initLexer(&lexer, source);
    tokenize(&lexer);
    
    if (debug) {
        printf("\nTOKENS:\n");
        for (int i = 0; i < lexer.token_count; i++) {
            Token t = lexer.tokens[i];
            printf("Token: %d | '%s'\n", t.type, t.lexeme);
        }
        printf("\n");
    }

    if (lexer.hadError) {
        printf("Error: %s\n", lexer.tokens[lexer.token_count - 2].lexeme);
        return VM_COMPILE_ERROR;
    }

    Parser parser;
    initParser(&parser, lexer.tokens);
    parse(&parser);

    if (debug) {
        printf("\nAST: \n");
        printAst(parser.ast);
        printf("\n");
    }

    if (parser.hadError) {
        return VM_COMPILE_ERROR;
    }

    Compiler compiler;
    initCompiler(&compiler, parser.ast);
    compile(&compiler);

    if (debug) {
        printf("\nBYTECODE: \n");
        for (int i = 0; i < compiler.bytecode->code_count; i++) {
            printf("OpCode: %d\n", compiler.bytecode->code[i]);
        }
        printf("\n");
    }

    freeParser(&parser);
    freeLexer(&lexer);

    initVm(vm, compiler.bytecode);

    while (vm->ip < vm->bytecode->code_count) {
        OpCode op = vm->bytecode->code[vm->ip++];
        evalOpCode(vm, op);
    }

    freeVm(vm);
    
    return VM_OK;
}