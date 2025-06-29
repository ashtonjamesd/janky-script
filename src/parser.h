#ifndef parser_h
#define parser_h

#include <stdbool.h>

#include "lexer.h"

typedef enum {
    AST_CONSTANT,
    AST_UNARY,
    AST_BINARY,
    
    AST_UNKNOWN,
} AstType;

typedef struct AstExpression AstExpression;

typedef struct {
    int dummy;
} UnknownExpression;

typedef struct {
    int value;
} ConstantExpression;

typedef struct {
    TokenType      op;
    AstExpression *right;
} UnaryExpression;

typedef struct {
    AstExpression *left;
    TokenType      op;
    AstExpression *right;
} BinaryExpression;

struct AstExpression {
    AstType type;

    union {
        UnaryExpression    unary;
        BinaryExpression   binary;
        ConstantExpression constant;
        UnknownExpression  unknown;
    } as;
};

typedef struct {
    AstExpression **exprs;
    int             expr_count;
    int             expr_capacity;
} Ast;

typedef struct {
    int    current;
    Token *tokens;
    Ast   *ast;
    bool   hadError;
} Parser;

void initParser(Parser *parser, Token *tokens);
void freeParser(Parser *parser);
void parse(Parser *Parser);

void printAst(Ast *ast);

#endif