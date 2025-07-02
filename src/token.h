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
    TRIPLE_EQUALS,
    TRIPLE_NOT_EQUALS,
    NOT_EQUALS,
    LESS_THAN,
    GREATER_THAN,
    LESS_THAN_EQUALS,
    GREATER_THAN_EQUALS,
    LOGICAL_NOT,
    LOGICAL_AND,
    LOGICAL_OR,
    BITWISE_AND,
    BITWISE_OR,
    BITWISE_XOR,
    BITWISE_NOT,
    BITWISE_LEFT_SHIFT,
    BITWISE_RIGHT_SHIFT,
    
    IF,
    ELSE,
    WHILE,
    RETURN,
    FUNCTION,
    VAR,
    LET,
    CONST,
    TYPEOF,

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