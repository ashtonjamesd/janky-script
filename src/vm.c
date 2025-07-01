#include <stdio.h>
#include <stdlib.h>

#include "vm.h"
#include "compiler.h"

void initVm(JankyVm *vm, Bytecode *bytecode) {
    vm->bytecode = bytecode;
    vm->ip = 0;
    vm->stack_top = vm->stack;
}

void freeVm(JankyVm *vm) {
    free(vm->bytecode->code);
    free(vm->bytecode->constants);
    free(vm->bytecode);
}

void push(JankyVm *vm, Value value) {
    // if (vm->stack_top >= STACK_MAX) {
    //     fprintf(stderr, "Stack overflow\n");
    //     exit(EXIT_FAILURE);
    // }
    *vm->stack_top = value;
    vm->stack_top++;
}

Value pop(JankyVm *vm) {
    // if (vm->stack_top == 0) {
    //     fprintf(stderr, "Stack underflow\n");
    //     exit(EXIT_FAILURE);
    // }
    vm->stack_top--;
    return *vm->stack_top;
}

static VmResult runtimeError(char *error) {
    printf("Error: %s\n", error);
    return VM_RUNTIME_ERROR;
}

static Value newNumber(int value) {
    Value number;
    number.type = TYPE_NUMBER;
    number.as.number = value;

    return number;
}

static Value newBoolean(bool value) {
    Value boolean;
    boolean.type = TYPE_BOOL;
    boolean.as.boolean = value;

    return boolean;
}

static VmResult evalOpCode(JankyVm *vm, OpCode op) {
    switch (op) {
        case OP_CONSTANT: {
            OpCode constIdx = vm->bytecode->code[vm->ip];
            vm->ip++;
            
            Value constant = vm->bytecode->constants[constIdx];
            push(vm, constant);

            break;
        }
        case OP_PLUS: {
            Value a = pop(vm);
            Value b = pop(vm);

            Value number = newNumber(a.as.number + b.as.number);
            push(vm, number);

            break;
        }
        case OP_MINUS: {
            Value a = pop(vm);
            Value b = pop(vm);

            Value number = newNumber(a.as.number - b.as.number);
            push(vm, number);

            break;
        }
        case OP_MULTIPLY: {
            Value a = pop(vm);
            Value b = pop(vm);

            Value number = newNumber(a.as.number * b.as.number);
            push(vm, number);

            break;
        }
        case OP_DIVIDE: {
            Value a = pop(vm);
            Value b = pop(vm);

            Value number = newNumber(a.as.number / b.as.number);
            push(vm, number);

            break;
        }
        case OP_MODULO: {
            Value a = pop(vm);
            Value b = pop(vm);

            int result = (int)a.as.number % (int)b.as.number;

            Value number = newNumber(result);
            push(vm, number);

            break;
        }
        case OP_NEGATE: {
            Value a = pop(vm);

            if (a.type != TYPE_NUMBER) {
                return runtimeError("Can only negate numeric values.");
            }

            Value number = newNumber(-a.as.number);
            push(vm, number);

            break;
        }
        case OP_NOT: {
            Value a = pop(vm);
            if (a.type != TYPE_BOOL) {
                return runtimeError("Can only apply logical not to boolean values.");
            }

            Value boolean = newBoolean(!a.as.boolean);
            push(vm, boolean);

            break;
        }
        case OP_END: {
            if (vm->stack_top == 0) {
                return VM_OK;
            }

            Value result = pop(vm);

            if (result.type == TYPE_BOOL) {
                printf("%s\n", result.as.boolean ? "true" : "false");
            } else {
                printf("%f\n", result.as.number);
            }

            break;
        }
        default: {
            fprintf(stderr, "Halting execution: unknown opcode '%d'\n", op);
            exit(EXIT_FAILURE);
        }
    }

    return VM_OK;
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
        VmResult result = evalOpCode(vm, op);
        
        if (result != VM_OK) {
            return result;
        }
    }

    freeVm(vm);
    
    return VM_OK;
}