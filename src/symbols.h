#ifndef symbols_h
#define symbols_h

#include "value.h"

typedef struct {
    char *key;
    Value value;
} Symbol;

typedef struct {
    Symbol *symbols;
    int     count;
    int     capacity;
} SymbolTable;

void initSymbolTable(SymbolTable *table);

#endif