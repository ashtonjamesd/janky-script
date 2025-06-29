#include <stdlib.h>
#include <stdio.h>

#include "parser.h"

static AstExpression *parseExpression(Parser *parser);

void initParser(Parser *parser, Token *tokens) {
    parser->tokens = tokens;
    parser->current = 0;
    parser->hadError = 0;

    parser->ast = malloc(sizeof(Ast));
    parser->ast->expr_capacity = 1;
    parser->ast->expr_count = 0;
    parser->ast->exprs = malloc(sizeof(AstExpression *) * parser->ast->expr_capacity);
}

void freeExpr(AstExpression *expr) {
    if (!expr) return;

    switch (expr->type) {
        case AST_UNARY:
            freeExpr(expr->as.unary.right);
            break;
        case AST_BINARY:
            freeExpr(expr->as.binary.left);
            freeExpr(expr->as.binary.right);
            break;
        default:
            break;
    }

    free(expr);
}


void freeParser(Parser *parser) {
    for (int i = 0; i < parser->ast->expr_count; i++) {
        AstExpression *expr = parser->ast->exprs[i];
        freeExpr(expr);
    }

    free(parser->ast->exprs);
    free(parser->ast);
}


static Token peek(Parser *parser) {
    return parser->tokens[parser->current];
}

static void advance(Parser *parser) {
    parser->current++;
}

static int isEnd(Parser *parser) {
    return peek(parser).type == TOKEN_EOF;
}

static void printExpr(AstExpression expr, int indent) {
    for (int i = 0; i < indent; i++) {
        printf(" ");
    }

    switch (expr.type) {
        case AST_UNARY: {
            printf("UNARY: ");
            printf("%s\n", token_type_to_str(expr.as.unary.op));
            printExpr(*expr.as.unary.right, indent + 2);
            break;
        }
        case AST_BINARY: {
            printf("BINARY: \n");
            printf("LEFT:");
            printExpr(*expr.as.binary.left, indent + 2);
            
            printf("OP: %s\n", token_type_to_str(expr.as.binary.op));
            
            printf("RIGHT:");
            printExpr(*expr.as.binary.right, indent + 2);
            break;
        }
        case AST_CONSTANT: {
            printf("CONSTANT: %d\n", expr.as.constant.value);
            break;
        }
        case AST_UNKNOWN: {
            printf("UNKNOWN\n");
            break;
        }
        default: {
            printf("OOPS bad expression..");
        }
    }
}

void printAst(Ast *ast) {
    for (int i = 0; i < ast->expr_count; i++) {
        AstExpression *expr = ast->exprs[i];
        printExpr(*expr, 0);
    }
}

static AstExpression *newExpr(AstType type) {
    AstExpression *expr = malloc(sizeof(AstExpression));
    expr->type = type;

    return expr;
}

static Token currentToken(Parser *parser) {
    if (isEnd(parser)) {
        Token token;
        token.type = TOKEN_EOF;
        return token;
    }

    return parser->tokens[parser->current];
}

static void parserError(Parser *parser, char *message) {
    parser->hadError = 1;
    printf("Error: %s\n", message);
}

static AstExpression *parsePrimary(Parser *parser) {
    Token token = currentToken(parser);
    advance(parser);

    switch (token.type) {
        case NUMBER: {
            AstExpression *expr = newExpr(AST_CONSTANT);
            expr->as.constant.value = atoi(token.lexeme);
            
            return expr;
        }
        default: {
            AstExpression *expr = newExpr(AST_UNKNOWN);
            expr->as.unknown.dummy = 0;

            parserError(parser, "Expected expression");
            return expr;
        }
    }
}

static int match(Parser *parser, TokenType type) {
    return isEnd(parser) ? 0 : parser->tokens[parser->current].type == type;
}

static AstExpression *parseUnary(Parser *parser) {
    while (match(parser, MINUS)) {
        TokenType op = parser->tokens[parser->current].type;
        advance(parser);

        AstExpression *right = parseUnary(parser);

        AstExpression *expr = newExpr(AST_UNARY);
        expr->as.unary.op = op;
        expr->as.unary.right = right;

        return expr;
    }

    return parsePrimary(parser);
}

static AstExpression *parseFactor(Parser *parser) {
    AstExpression *left = parseUnary(parser);

    while (match(parser, STAR) || match(parser, SLASH) || match(parser, MODULO)) {
        TokenType op = parser->tokens[parser->current].type;
        advance(parser);

        AstExpression *right = parseUnary(parser);

        AstExpression *expr = newExpr(AST_BINARY);
        expr->as.binary.left = left;
        expr->as.binary.op = op;
        expr->as.binary.right = right;

        left = expr;
    }

    return left;
}

static AstExpression *parseTerm(Parser *parser) {
    AstExpression *left = parseFactor(parser);

    while (match(parser, PLUS) || match(parser, MINUS)) {
        TokenType op = parser->tokens[parser->current].type;
        advance(parser);

        AstExpression *right = parseFactor(parser);

        AstExpression *expr = newExpr(AST_BINARY);
        expr->as.binary.left = left;
        expr->as.binary.op = op;
        expr->as.binary.right = right;

        left = expr;
    }

    return left;
}

static AstExpression *parseExpression(Parser *parser) {
    return parseTerm(parser);
}

void parse(Parser *parser) {
    while (!isEnd(parser)) {
        AstExpression* expr = parseExpression(parser);


        if (parser->ast->expr_count >= parser->ast->expr_capacity) {
            parser->ast->expr_capacity *= 2;
            parser->ast->exprs = realloc(parser->ast->exprs, parser->ast->expr_capacity * sizeof(AstExpression *));
        }
        parser->ast->exprs[parser->ast->expr_count++] = expr;

        if (parser->hadError) {
            return;
        }
    }
}