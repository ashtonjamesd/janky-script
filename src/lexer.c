#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "lexer.h"

void initLexer(Lexer *lexer, char *source) {
    lexer->source = strdup(source);
    lexer->current = lexer->source;
    lexer->start = lexer->source;
    lexer->hadError = false;

    lexer->token_capacity = 1;
    lexer->token_count = 0;
    lexer->tokens = malloc(sizeof(Token) * lexer->token_capacity);
}

void freeLexer(Lexer *lexer) {
    for (int i = 0; i < lexer->token_count; i++) {
        free(lexer->tokens[i].lexeme);
    }
    free(lexer->tokens);
    free(lexer->source);
}

static void advance(Lexer *lexer) {
    lexer->current++;
}

static int isDigit(char c) {
    return c >= '0' && c <= '9';
}

static int isLetter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static int isEnd(Lexer *lexer) {
    return *lexer->current == '\0';
}

static char peek(Lexer *lexer) {
  return *lexer->current;
}

static Token newToken(Lexer *lexer, TokenType type) {
    Token token;

    size_t length = lexer->current - lexer->start;
    token.lexeme = malloc(length + 1);
    memcpy(token.lexeme, lexer->start, length);

    token.lexeme[length] = '\0';
    token.type = type;

    return token;
}

static Token compileErrorToken(Lexer *lexer, char *message) {
    lexer->hadError = true;

    Token token;
    token.lexeme = message;
    token.type = BAD;
    return token;
}

static Token tokenizeString(Lexer *lexer) {
    advance(lexer);

    lexer->start = lexer->current;
    while (!isEnd(lexer) && peek(lexer) != '\"') {
        advance(lexer);
    }
    if (isEnd(lexer)) {
        return compileErrorToken(lexer, "Unterminated string literal.");
    }

    size_t length = lexer->current - lexer->start;
    Token token;
    token.lexeme = malloc(length + 1);
    memcpy(token.lexeme, lexer->start, length);
    token.lexeme[length] = '\0';
    token.type = STRING;

    advance(lexer);

    return token;
}

static Token tokenizeNumber(Lexer *lexer) {
    bool hasDecimal = false;
    while (!isEnd(lexer) && (isDigit(peek(lexer)) || peek(lexer) == '.')) {
        if (peek(lexer) == '.' && !hasDecimal) {
            hasDecimal = true;
        } else if (peek(lexer) == '.') {
            return compileErrorToken(lexer, "Invalid numeric literal.");
        }

        advance(lexer);
    }
    return newToken(lexer, NUMBER);
}

static TokenType getIdentifierType(Lexer *lexer) {
    size_t length = lexer->current - lexer->start;
    char *lexeme = malloc(length + 1);
    memcpy(lexeme, lexer->start, length);
    lexeme[length] = '\0';
    
    if (strcmp("let", lexeme) == 0) {
        return LET;
    } else if (strcmp("true", lexeme) == 0) {
        return TRUE;
    } else if (strcmp("false", lexeme) == 0) {
        return FALSE;
    } else if (strcmp("typeof", lexeme) == 0) {
        return TYPEOF;
    } else if (strcmp("var", lexeme) == 0) {
        return VAR;
    } else if (strcmp("const", lexeme) == 0) {
        return CONST;
    }

    free(lexeme);

    return IDENTIFIER;
}

static Token tokenizeIdentifier(Lexer *lexer) {
    while (!isEnd(lexer) && isLetter(peek(lexer))) {
        advance(lexer);
    }

    TokenType type = getIdentifierType(lexer);
    return newToken(lexer, type);
}

static Token nextToken(Lexer *lexer) {
    lexer->start = lexer->current;

    char c = *lexer->current;

    if (isDigit(c)) return tokenizeNumber(lexer);
    if (c == '\"') return tokenizeString(lexer);
    if (isLetter(c)) return tokenizeIdentifier(lexer);

    switch (c) {
        case '=': {
            advance(lexer);
            if (peek(lexer) == '=') {
                advance(lexer);
                
                if (peek(lexer) == '=') {
                    advance(lexer);
                    return newToken(lexer, TRIPLE_EQUALS);
                }
                
                return newToken(lexer, DOUBLE_EQUALS);
            }
            return newToken(lexer, SINGLE_EQUALS);
        }
        case '!': {
            advance(lexer);
            if (peek(lexer) == '=') {
                advance(lexer);
                                
                if (peek(lexer) == '=') {
                    advance(lexer);
                    return newToken(lexer, TRIPLE_NOT_EQUALS);
                }

                return newToken(lexer, NOT_EQUALS);
            }
            return newToken(lexer, LOGICAL_NOT);
        }
        case '&': {
            advance(lexer);
            if (peek(lexer) == '&') {
                advance(lexer);
                return newToken(lexer, LOGICAL_AND);
            }
            return newToken(lexer, BITWISE_AND);
        }
        case '|': {
            advance(lexer);
            if (peek(lexer) == '|') {
                advance(lexer);
                return newToken(lexer, LOGICAL_OR);
            }
            return newToken(lexer, BITWISE_OR);
        }
        case '>': {
            advance(lexer);

            if (peek(lexer) == '=') {
                advance(lexer);
                return newToken(lexer, GREATER_THAN_EQUALS);
            }
            if (peek(lexer) == '>') {
                advance(lexer);
                return newToken(lexer, BITWISE_RIGHT_SHIFT);
            }

            return newToken(lexer, GREATER_THAN);
        }
        case '<': {
            advance(lexer);

            if (peek(lexer) == '=') {
                advance(lexer);
                return newToken(lexer, LESS_THAN_EQUALS);
            }
            if (peek(lexer) == '<') {
                advance(lexer);
                return newToken(lexer, BITWISE_LEFT_SHIFT);
            }

            return newToken(lexer, LESS_THAN);
        }
        case '~': {
            advance(lexer);
            return newToken(lexer, BITWISE_NOT);
        }
        case '^': {
            advance(lexer);
            return newToken(lexer, BITWISE_XOR);
        }
        case ';': {
            advance(lexer);
            return newToken(lexer, SEMICOLON);
        }
        case '+': {
            advance(lexer);
            return newToken(lexer, PLUS);
        }
        case '-': {
            advance(lexer);
            return newToken(lexer, MINUS);
        }
        case '*': {
            advance(lexer);
            return newToken(lexer, STAR);
        }
        case '/': {
            advance(lexer);
            return newToken(lexer, SLASH);
        }
        case '%': {
            advance(lexer);
            return newToken(lexer, MODULO);
        }
    }

    advance(lexer);
    return compileErrorToken(lexer, "Unexpected character.");
}

static void skipWhitespace(Lexer *lexer) {
    for (;;) {
        switch (peek(lexer)) {
            case ' ':
            case '\r':
            case '\t':
            case '\n':
                advance(lexer);
                break;
            default:
                return;
        }
    }
}

void addToken(Lexer *lexer, Token token) {
    if (lexer->token_count >= lexer->token_capacity) {
        lexer->token_capacity *= 2;
        Token *newTokens = realloc(lexer->tokens, lexer->token_capacity * sizeof(Token));
        if (!newTokens) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
        lexer->tokens = newTokens;
    }

    lexer->tokens[lexer->token_count++] = token;
}

void tokenize(Lexer *lexer) {
    while (!isEnd(lexer)) {
        skipWhitespace(lexer);
        if (isEnd(lexer)) break;

        Token token = nextToken(lexer);
        addToken(lexer, token);

        if (lexer->hadError) break;
    }

    Token token = newToken(lexer, TOKEN_EOF);
    addToken(lexer, token);
}