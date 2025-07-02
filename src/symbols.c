#include <stdio.h>
#include <stdint.h>

#include "symbols.h"

void initSymbolTable(SymbolTable *table) {
    table->count = 0;
  table->capacity = 0;
  table->symbols = NULL;
}

static uint32_t hashString(const char* key, int length) {
    uint32_t hash = 2166136261u;
    
    for (int i = 0; i < length; i++) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }

    return hash;
}