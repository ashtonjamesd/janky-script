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

static Token errorToken(char *message) {
    Token token;
    token.lexeme = message;
    token.type = BAD;
    return token;
}

static Token tokenizeString(Lexer *lexer) {
    advance(lexer);
    while (!isEnd(lexer) && peek(lexer) != '\"') {
        advance(lexer);
    }
    if (isEnd(lexer)) {
        lexer->hadError = true;
        return errorToken("Unterminated string literal.");
    }

    advance(lexer);
    return newToken(lexer, STRING);
}

static Token tokenizeNumber(Lexer *lexer) {
    while (!isEnd(lexer) && isDigit(peek(lexer))) {
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
                return newToken(lexer, DOUBLE_EQUALS);
            }
            return newToken(lexer, SINGLE_EQUALS);
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
    return errorToken("Unexpected character.");
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
            exit(1);
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