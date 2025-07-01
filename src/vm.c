#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static bool valuesEqual(Value a, Value b) {
    if (a.type != b.type) return false;

    switch (a.type) {
        case TYPE_NUMBER:
            return a.as.number == b.as.number;
        case TYPE_BOOL:
            return a.as.boolean == b.as.boolean;
        case TYPE_STRING:
            return strcmp(a.as.object->as.string.chars, b.as.object->as.string.chars) == 0;
        default:
            fprintf(stderr, "Unknown type in valuesEqual, exiting.\n");
            exit(EXIT_FAILURE);
    }
}

static bool looselyEqual(Value a, Value b) {
    if (a.type == b.type) {
        return valuesEqual(a, b);
    }

    if (a.type == TYPE_BOOL && b.type == TYPE_NUMBER) {
        return (a.as.boolean ? 1 : 0) == b.as.number;
    }
    if (a.type == TYPE_NUMBER && b.type == TYPE_BOOL) {
        return a.as.number == (b.as.boolean ? 1 : 0);
    }
    if (a.type == TYPE_STRING && b.type == TYPE_NUMBER) {
        return atoi(a.as.object->as.string.chars) == b.as.number;
    }
    if (a.type == TYPE_NUMBER && b.type == TYPE_STRING) {
        return atoi(b.as.object->as.string.chars) == a.as.number;
    }
    if (a.type == TYPE_BOOL && b.type == TYPE_STRING) {
        return atoi(b.as.object->as.string.chars) == a.as.boolean;
    }
    if (a.type == TYPE_STRING && b.type == TYPE_BOOL) {
        return atoi(a.as.object->as.string.chars) == b.as.boolean;
    }

    return false;
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
        case OP_LOGICAL_NOT: {
            Value a = pop(vm);
            if (a.type != TYPE_BOOL) {
                return runtimeError("Can only apply logical not to boolean values.");
            }

            Value boolean = newBoolean(!a.as.boolean);
            push(vm, boolean);

            break;
        }
        case OP_LOGICAL_AND: {
            Value a = pop(vm);
            Value b = pop(vm);
            
            if (a.type != TYPE_BOOL || b.type != TYPE_BOOL) {
                return runtimeError("Can only apply logical and to boolean values.");
            }

            Value boolean = newBoolean(a.as.boolean && b.as.boolean);
            push(vm, boolean);
            
            break;
        }
        case OP_LOGICAL_OR: {
            Value a = pop(vm);
            Value b = pop(vm);
            
            if (a.type != TYPE_BOOL || b.type != TYPE_BOOL) {
                return runtimeError("Can only apply logical or to boolean values.");
            }

            Value boolean = newBoolean(a.as.boolean || b.as.boolean);
            push(vm, boolean);
            
            break;
        }
        case OP_BITWISE_AND: {
            Value a = pop(vm);
            Value b = pop(vm);
            
            if (a.type != TYPE_NUMBER || b.type != TYPE_NUMBER) {
                return runtimeError("Can only apply bitwise and to number values.");
            }

            Value number = newNumber((int)a.as.number & (int)b.as.number);
            push(vm, number);
            
            break;
        }
        case OP_BITWISE_NOT: {
            Value a = pop(vm);
            
            if (a.type != TYPE_NUMBER) {
                return runtimeError("Can only apply bitwise not to number values.");
            }

            Value number = newNumber(~(int)a.as.number );
            push(vm, number);
            
            break;
        }
        case OP_BITWISE_XOR: {
            Value a = pop(vm);
            Value b = pop(vm);
            
            if (a.type != TYPE_NUMBER || b.type != TYPE_NUMBER) {
                return runtimeError("Can only apply bitwise xor to number values.");
            }

            Value number = newNumber((int)a.as.number ^ (int)b.as.number);
            push(vm, number);
            
            break;
        }
        case OP_BITWISE_LEFT_SHIFT: {
            Value a = pop(vm);
            Value b = pop(vm);
            
            if (a.type != TYPE_NUMBER || b.type != TYPE_NUMBER) {
                return runtimeError("Can only apply bitwise xor to number values.");
            }

            Value number = newNumber((int)a.as.number << (int)b.as.number);
            push(vm, number);
            
            break;
        }
        case OP_BITWISE_RIGHT_SHIFT: {
            Value a = pop(vm);
            Value b = pop(vm);
            
            if (a.type != TYPE_NUMBER || b.type != TYPE_NUMBER) {
                return runtimeError("Can only apply bitwise xor to number values.");
            }

            Value number = newNumber((int)a.as.number >> (int)b.as.number);
            push(vm, number);
            
            break;
        }
        case OP_EQUALS: {
            Value b = pop(vm);
            Value a = pop(vm);

            push(vm, newBoolean(looselyEqual(a, b)));
            break;
        }
        case OP_NOT_EQUALS: {
            Value b = pop(vm);
            Value a = pop(vm);

            push(vm, newBoolean(!looselyEqual(a, b)));
            break;
        }
        case OP_TRIPLE_EQUALS: {
            Value a = pop(vm);
            Value b = pop(vm);
            
            if (a.type != b.type) {
                push(vm, newBoolean(false));
                break;
            }

            Value result;
            if (a.type == TYPE_NUMBER) {
                result = newBoolean(a.as.number == b.as.number);
            } else if (a.type == TYPE_BOOL) {
                result = newBoolean(a.as.boolean == b.as.boolean);
            } else if (a.type == TYPE_STRING) {
                result = newBoolean(strcmp(a.as.object->as.string.chars, b.as.object->as.string.chars) == 0);
            } else {
                fprintf(stderr, "Unknown type in vm, exiting.\n");
                exit(EXIT_FAILURE);
            }

            push(vm, result);
            
            break;
        }
        case OP_TRIPLE_NOT_EQUALS: {
            Value a = pop(vm);
            Value b = pop(vm);
            
            if (a.type != b.type) {
                push(vm, newBoolean(false));
                break;
            }

            Value result;
            if (a.type == TYPE_NUMBER) {
                result = newBoolean(a.as.number == b.as.number);
            } else if (a.type == TYPE_BOOL) {
                result = newBoolean(a.as.boolean == b.as.boolean);
            } else if (a.type == TYPE_STRING) {
                result = newBoolean(strcmp(a.as.object->as.string.chars, b.as.object->as.string.chars) == 0);
            } else {
                fprintf(stderr, "Unknown type in vm, exiting.\n");
                exit(EXIT_FAILURE);
            }

            push(vm, result);
            
            break;
        }
        case OP_LESS_THAN: {
            Value a = pop(vm);
            Value b = pop(vm);
            
            if (a.type != b.type) {
                return runtimeError("Can only apply less than to number values.");
            }

            Value result = newBoolean(a.as.number < b.as.number);
            push(vm, result);
            
            break;
        }
        case OP_LESS_THAN_EQUALS: {
            Value a = pop(vm);
            Value b = pop(vm);
            
            if (a.type != b.type) {
                return runtimeError("Can only apply less than equals to number values.");
            }

            Value result = newBoolean(a.as.number <= b.as.number);
            push(vm, result);
            
            break;
        }
        case OP_GREATER_THAN: {
            Value a = pop(vm);
            Value b = pop(vm);
            
            if (a.type != b.type) {
                return runtimeError("Can only apply greater than to number values.");
            }

            Value result = newBoolean(a.as.number > b.as.number);
            push(vm, result);
            
            break;
        }
        case OP_GREATER_THAN_EQUALS: {
            Value a = pop(vm);
            Value b = pop(vm);
            
            if (a.type != b.type) {
                return runtimeError("Can only apply greater than to number values.");
            }

            Value result = newBoolean(a.as.number >= b.as.number);
            push(vm, result);
            
            break;
        }
        case OP_END: {
            if (vm->stack_top == 0) {
                return VM_OK;
            }

            Value result = pop(vm);

            if (result.type == TYPE_BOOL) {
                printf("%s\n", result.as.boolean ? "true" : "false");
            } else if (result.type == TYPE_NUMBER) {
                printf("%f\n", result.as.number);
            } else if (result.type == TYPE_STRING) {
                printf("%s\n", result.as.object->as.string.chars);
            }
            else {
                fprintf(stderr, "Unknown end result type in vm.\n");
                exit(EXIT_FAILURE);
            }

            break;
        }
        default: {
            fprintf(stderr, "Halting VM execution: unknown opcode '%d'\n", op);
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