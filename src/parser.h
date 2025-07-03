#ifndef parser_h
#define parser_h

#include <stdbool.h>

#include "lexer.h"
#include "value.h"

typedef enum {
    AST_CONSTANT,
    AST_UNARY,
    AST_BINARY,
    AST_VARIABLE_DECLARATION,
    AST_PROPERTY,
    AST_CALL,
    
    AST_UNKNOWN,
} AstType;

typedef struct AstExpression AstExpression;

typedef struct {
    int dummy;
} UnknownExpression;

typedef struct {
    ValueType type;
    union {
        int     number;
        bool    boolean;
        Object *object;
        char   *identifier;
    } as;
} ConstantExpression;

typedef enum {
    VARIABLE_LET,
    VARIABLE_VAR,
    VARIABLE_CONST,
    VARIABLE_UNKNOWN,
} VariableBinding;

typedef struct {
    AstExpression *object;
    char          *property;
} AstProperty;

typedef struct {
    char *identifier;
    VariableBinding binding;
    AstExpression *initializer;
} VariableDeclaration;

typedef struct {
    AstExpression **args;
    int             argCount;
    AstExpression  *callee;
} AstCall;

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
        UnaryExpression     unary;
        BinaryExpression    binary;
        ConstantExpression  constant;
        UnknownExpression   unknown;
        VariableDeclaration variable;
        AstCall             call;
        AstProperty         property;
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