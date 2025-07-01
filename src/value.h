#ifndef value_h
#define value_h

typedef enum {
    TYPE_BOOL,
    TYPE_NUMBER,
    TYPE_STRING,
} ValueType;

typedef struct {
    ValueType type;
    union {
        bool   boolean;
        double number;
    } as;
} Value;

#endif