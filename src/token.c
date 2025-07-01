#include "token.h"

char *token_type_to_str(TokenType type) {
    switch (type) {
        case NUMBER_TYPE: return "NUMBER_TYPE";
        case BOOLEAN_TYPE: return "BOOLEAN_TYPE";
        case UNDEFINED: return "UNDEFINED";
        case NUMBER: return "NUMBER";
        case BOOLEAN: return "BOOLEAN";
        case IDENTIFIER: return "IDENTIFIER";
        case STRING: return "STRING";
        case TRUE: return "TRUE";
        case FALSE: return "FALSE";
        case PLUS: return "PLUS";
        case MINUS: return "MINUS";
        case STAR: return "STAR";
        case SLASH: return "SLASH";
        case MODULO: return "MODULO";
        case LEFT_PAREN: return "LEFT_PAREN";
        case RIGHT_PAREN: return "RIGHT_PAREN";
        case LEFT_BRACE: return "LEFT_BRACE";
        case RIGHT_BRACE: return "RIGHT_BRACE";
        case SEMICOLON: return "SEMICOLON";
        case COMMA: return "COMMA";
        case SINGLE_EQUALS: return "SINGLE_EQUALS";
        case DOUBLE_EQUALS: return "DOUBLE_EQUALS";
        case IF: return "IF";
        case ELSE: return "ELSE";
        case WHILE: return "WHILE";
        case RETURN: return "RETURN";
        case FUNCTION: return "FUNCTION";
        case VAR: return "VAR";
        case LET: return "LET";
        case CONST: return "CONST";
        case TOKEN_EOF: return "TOKEN_EOF";
        case BAD: return "BAD";
        default: return "UNKNOWN_TOKEN";
    }
}