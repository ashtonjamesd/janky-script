#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
        case AST_CONSTANT:
            if (expr->as.constant.type == TYPE_IDENTIFIER) {
                free(expr->as.constant.as.identifier);
            }
            break;
        case AST_VARIABLE_DECLARATION:
            free(expr->as.variable.identifier);

            if (expr->as.variable.initializer) {
                free(expr->as.variable.initializer);
            }
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

static inline void advance(Parser *parser) {
    parser->current++;
}

static inline Token peek(Parser *parser) {
    return parser->tokens[parser->current];
}

static inline int isEnd(Parser *parser) {
    return peek(parser).type == TOKEN_EOF;
}

static int match(Parser *parser, TokenType type) {
    return isEnd(parser) ? 0 : parser->tokens[parser->current].type == type;
}

static inline bool expect(Parser *parser, TokenType type) {
    if (match(parser, type)) {
        advance(parser);
        return true;
    }

    return false;
}

static Token currentToken(Parser *parser) {
    if (isEnd(parser)) {
        Token token;
        token.type = TOKEN_EOF;
        return token;
    }

    return parser->tokens[parser->current];
}

static AstExpression *compileError(Parser *parser, char *message) {
    parser->hadError = 1;
    printf("Error: %s\n", message);

    return NULL;
}

static bool expectIdentifier(Parser *parser) {
    if (!expect(parser, IDENTIFIER)) {
        compileError(parser, "Expected identifier");
        return false;
    }

    return true;
}

static void printIndent(int indent) {
    for (int i = 0; i < indent; ++i) {
        putchar(' ');
    }
}

static void printExpr(AstExpression expr, int indent) {
    switch (expr.type) {

    case AST_CONSTANT:
        printIndent(indent);
        switch (expr.as.constant.type) {
            case TYPE_NUMBER:   printf("NUMBER   : %d\n",  expr.as.constant.as.number);          break;
            case TYPE_BOOL:     printf("BOOLEAN  : %s\n",  expr.as.constant.as.boolean ? "true":"false"); break;
            case TYPE_STRING:   printf("STRING   : \"%s\"\n", expr.as.constant.as.object->as.string.chars); break;
            case TYPE_IDENTIFIER:
                               printf("IDENT    : %s\n",   expr.as.constant.as.identifier);     break;
            default:            printf("CONST ?  \n");                                           break;
        }
        break;

    case AST_UNARY:
        printIndent(indent); printf("UNARY     : %s\n", token_type_to_str(expr.as.unary.op));
        printExpr(*expr.as.unary.right, indent + 2);
        break;

    case AST_BINARY:
        printIndent(indent); printf("BINARY    : %s\n", token_type_to_str(expr.as.binary.op));
        printIndent(indent); printf("LEFT  ->\n");
        printExpr(*expr.as.binary.left,  indent + 4);
        printIndent(indent); printf("RIGHT ->\n");
        printExpr(*expr.as.binary.right, indent + 4);
        break;

    case AST_PROPERTY:
        printIndent(indent); printf("PROPERTY  : .%s\n", expr.as.property.property);
        printIndent(indent); printf("OBJECT ->\n");
        printExpr(*expr.as.property.object, indent + 4);
        break;

    case AST_CALL:
        printIndent(indent); printf("CALL      : (%d arg%s)\n",
                              expr.as.call.argCount,
                              expr.as.call.argCount == 1 ? "" : "s");
        printIndent(indent); printf("CALLEE ->\n");
        printExpr(*expr.as.call.callee, indent + 4);

        for (int i = 0; i < expr.as.call.argCount; ++i) {
            printIndent(indent); printf("ARG[%d] ->\n", i);
            printExpr(*expr.as.call.args[i], indent + 6);
        }
        break;

    case AST_VARIABLE_DECLARATION:
        printIndent(indent);
        printf("VAR_DECL  : %d %s\n",
               expr.as.variable.binding,
               expr.as.variable.identifier);
        if (expr.as.variable.initializer) {
            printIndent(indent); printf("INIT ->\n");
            printExpr(*expr.as.variable.initializer, indent + 4);
        }
        break;

    case AST_UNKNOWN:
        printIndent(indent); printf("UNKNOWN\n");
        break;

    default:
        printIndent(indent); printf("<<unhandled expr.type %d>>\n", expr.type);
        break;
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

AstExpression *newBinaryExpr(TokenType op, AstExpression *left, AstExpression *right) {
    AstExpression *expr = newExpr(AST_BINARY);
    expr->as.binary.left = left;
    expr->as.binary.op = op;
    expr->as.binary.right = right;
    
    return expr;
}

static AstExpression *parsePrimary(Parser *parser) {
    Token token = currentToken(parser);
    advance(parser);

    switch (token.type) {
        case NUMBER: {
            AstExpression *expr = newExpr(AST_CONSTANT);
            expr->as.constant.type = TYPE_NUMBER;
            expr->as.constant.as.number = atoi(token.lexeme);
            
            return expr;
        }
        case TRUE:
        case FALSE: {
            AstExpression *expr = newExpr(AST_CONSTANT);
            expr->as.constant.type = TYPE_BOOL;
            expr->as.constant.as.boolean = strcmp(token.lexeme, "true") == 0;

            return expr;
        }
        case STRING: {
            AstExpression *expr = newExpr(AST_CONSTANT);
            expr->as.constant.type = TYPE_STRING;

            Object *obj = malloc(sizeof(Object));
            obj->type = OBJ_STRING;

            expr->as.constant.as.object = obj;

            expr->as.constant.as.object->as.string.chars = strdup(token.lexeme);
            expr->as.constant.as.object->as.string.length = strlen(token.lexeme);

            return expr;
        }
        case IDENTIFIER: {
            AstExpression *expr = newExpr(AST_CONSTANT);
            expr->as.constant.type = TYPE_IDENTIFIER;
            expr->as.constant.as.identifier = strdup(token.lexeme);
            
            return expr;
        }
        default: {
            compileError(parser, "Expected expression");
            return NULL;
        }
    }
}

static AstExpression *parsePostfix(Parser *parser) {
    AstExpression *expr = parsePrimary(parser);

    for (;;) {
        if (match(parser, DOT)) {
            advance(parser);

            Token name = currentToken(parser);
            if (!expectIdentifier(parser)) return compileError(parser, "Expected property name after '.");
            
            AstExpression *prop = newExpr(AST_PROPERTY);
            prop->as.property.object  = expr;
            prop->as.property.property = strdup(name.lexeme);
            expr = prop;
        }
    }

    return expr;
}

static AstExpression *parseUnary(Parser *parser) {
    while (match(parser, MINUS) || match(parser, LOGICAL_NOT) || match(parser, BITWISE_NOT) || match(parser, TYPEOF)) {
        TokenType op = currentToken(parser).type;
        advance(parser);

        AstExpression *right = parseUnary(parser);

        AstExpression *expr = newExpr(AST_UNARY);
        expr->as.unary.op = op;
        expr->as.unary.right = right;

        return expr;
    }

    return parsePostfix(parser);
}

static AstExpression *parseFactor(Parser *parser) {
    AstExpression *left = parseUnary(parser);

    while (match(parser, STAR) || match(parser, SLASH) || match(parser, MODULO)) {
        TokenType op = currentToken(parser).type;
        advance(parser);

        AstExpression *right = parseUnary(parser);
        left = newBinaryExpr(op, left, right);
    }

    return left;
}

static AstExpression *parseTerm(Parser *parser) {
    AstExpression *left = parseFactor(parser);

    while (match(parser, PLUS) || match(parser, MINUS)) {
        TokenType op = currentToken(parser).type;
        advance(parser);

        AstExpression *right = parseFactor(parser);
        left = newBinaryExpr(op, left, right);
    }

    return left;
}

static AstExpression *parseShifts(Parser *parser) {
    AstExpression *left = parseTerm(parser);

    while (match(parser, BITWISE_RIGHT_SHIFT) || match(parser, BITWISE_LEFT_SHIFT)) {
        TokenType op = currentToken(parser).type;
        advance(parser);

        AstExpression *right = parseTerm(parser);
        left = newBinaryExpr(op, left, right);
    }

    return left;
}

static AstExpression *parseComparison(Parser *parser) {
    AstExpression *left = parseShifts(parser);

    while (match(parser, DOUBLE_EQUALS) || match(parser, TRIPLE_EQUALS) || match(parser, TRIPLE_NOT_EQUALS) 
        || match(parser, NOT_EQUALS) || match(parser, GREATER_THAN) || match(parser, LESS_THAN) 
        || match(parser, GREATER_THAN_EQUALS) || match(parser, LESS_THAN_EQUALS)
    ) {
        TokenType op = currentToken(parser).type;
        advance(parser);

        AstExpression *right = parseShifts(parser);
        left = newBinaryExpr(op, left, right);
    }

    return left;
}

static AstExpression *parseBitwiseAnd(Parser *parser) {
    AstExpression *left = parseComparison(parser);

    while (match(parser, BITWISE_AND)) {
        TokenType op = currentToken(parser).type;
        advance(parser);

        AstExpression *right = parseComparison(parser);
        left = newBinaryExpr(op, left, right);
    }

    return left;
}

static AstExpression *parseBitwiseXor(Parser *parser) {
    AstExpression *left = parseBitwiseAnd(parser);

    while (match(parser, BITWISE_XOR)) {
        TokenType op = currentToken(parser).type;
        advance(parser);

        AstExpression *right = parseBitwiseAnd(parser);
        left = newBinaryExpr(op, left, right);
    }

    return left;
}

static AstExpression *parseBitwiseOr(Parser *parser) {
    AstExpression *left = parseBitwiseXor(parser);

    while (match(parser, BITWISE_OR)) {
        TokenType op = currentToken(parser).type;
        advance(parser);

        AstExpression *right = parseBitwiseXor(parser);
        left = newBinaryExpr(op, left, right);
    }

    return left;
}

static AstExpression *parseAnd(Parser *parser) {
    AstExpression *left = parseBitwiseOr(parser);

    while (match(parser, LOGICAL_AND)) {
        TokenType op = currentToken(parser).type;
        advance(parser);

        AstExpression *right = parseBitwiseOr(parser);
        left = newBinaryExpr(op, left, right);
    }

    return left;
}

static AstExpression *parseOr(Parser *parser) {
    AstExpression *left = parseAnd(parser);

    while (match(parser, LOGICAL_OR)) {
        TokenType op = currentToken(parser).type;
        advance(parser);

        AstExpression *right = parseAnd(parser);
        left = newBinaryExpr(op, left, right);
    }

    return left;
}

static AstExpression *parseExpression(Parser *parser) {
    return parseOr(parser);
}

static inline VariableBinding mapVariableBinding(TokenType type) {
    switch(type) {
        case LET: return VARIABLE_LET;
        case VAR: return VARIABLE_VAR;
        case CONST: return VARIABLE_CONST;
        default: return VARIABLE_UNKNOWN;
    }
}

static bool expectSemicolon(Parser *parser) {
    if (!expect(parser, SEMICOLON)) {
        compileError(parser, "Expected ';'");
        return false;
    }

    return true;
}

static inline AstExpression *noExpr() {
    return NULL;
}

static AstExpression *parseVariableDeclaration(Parser *parser) {
    Token declType = currentToken(parser);
    advance(parser);

    Token identifier = currentToken(parser);
    if (!expectIdentifier(parser)) return noExpr();

    AstExpression *initializer = NULL;
    
    if (match(parser, SEMICOLON)) {
        advance(parser);
        goto makeStmt;
    }

    if (!expect(parser, SINGLE_EQUALS)) {
        return compileError(parser, "Expected '=' or ';");
    }

    initializer = parseExpression(parser);
    if (!initializer) return NULL;

    if (!expectSemicolon(parser)) return noExpr();

    makeStmt:
    AstExpression *stmt = newExpr(AST_VARIABLE_DECLARATION);
    
    stmt->as.variable.binding = mapVariableBinding(declType.type);
    stmt->as.variable.identifier = strdup(identifier.lexeme);
    stmt->as.variable.initializer = initializer;

    return stmt;
}

static AstExpression *parseStatement(Parser *parser) {
    if (match(parser, LET) || match(parser, CONST) || match(parser, VAR)) {
        return parseVariableDeclaration(parser);
    }

    return parseExpression(parser);
}

void parse(Parser *parser) {
    while (!isEnd(parser)) {
        AstExpression* expr = parseStatement(parser);

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