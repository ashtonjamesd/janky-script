#ifndef lexer_h
#define lexer_h

#include <stdbool.h>

#include "token.h"

typedef struct {
    char  *source;
    char  *start;
    char  *current;
    int    token_count;
    int    token_capacity;
    Token *tokens;
    bool   hadError;
} Lexer;

void initLexer(Lexer *lexer, char *source);
void freeLexer(Lexer *lexer);
void tokenize(Lexer *lexer);

#endif