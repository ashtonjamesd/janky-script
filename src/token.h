#ifndef token_h
#define token_h


typedef enum {
    NUMBER_TYPE,
    BOOLEAN_TYPE,
    UNDEFINED,

    NUMBER,
    BOOLEAN,
    IDENTIFIER,
    STRING,
    TRUE,
    FALSE,
    
    PLUS,
    MINUS,
    STAR,
    SLASH,
    MODULO,

    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    SEMICOLON,
    COMMA,
    SINGLE_EQUALS,
    DOUBLE_EQUALS,
    NOT_EQUALS,
    NOT,
    
    IF,
    ELSE,
    WHILE,
    RETURN,
    FUNCTION,
    VAR,
    LET,
    CONST,

    TOKEN_EOF,
    BAD
} TokenType;

typedef struct {
    TokenType type;
    char     *lexeme;
    int       length;
} Token;

char *token_type_to_str(TokenType type);

#endif