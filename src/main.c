#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "vm.h"

char *readFile(char *path) {
    FILE *fptr = fopen(path, "r");
    if (!fptr) {
        fprintf(stderr, "Error opening source file\n");
        return NULL;
    }

    fseek(fptr, 0, SEEK_END);
    long sz = ftell(fptr);
    rewind(fptr);

    char *buffer = malloc(sz + 1);
    if (!buffer) {
        fprintf(stderr, "Error allocating buffer for source file...\n");
        fclose(fptr);
        return NULL;
    }
    
    fread(buffer, 1, sz, fptr);
    fclose(fptr);

    buffer[sz] = '\0';

    return buffer;
}

void repl(int debug) {
    char line[1024];

    for (;;) {
        printf("janky-vm>  ");
        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        JankyVm vm;
        VmResult result = run(&vm, line, debug);

        if (result == VM_COMPILE_ERROR) {
            printf("Compile time error.\n");
        } 
        else if (result == VM_RUNTIME_ERROR) {
            printf("Runtime error.\n");
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: ./jank <source_path.js>\n");
        return 1;
    }

    int replMode = 0;
    int debug = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp("--repl", argv[i]) == 0) replMode = 1;
        else if (strcmp("--debug", argv[i]) == 0) debug = 1;
        else {
            if (replMode) {
                printf("Unknown flag '%s'", argv[i]);
                return 1;
            }
        }
    }

    if (replMode) {
        repl(debug);
    } else {
        char *buffer = readFile(argv[1]);
        if (!buffer) return 1;

        JankyVm vm;
        VmResult result = run(&vm, buffer, debug);
        free(buffer);

        if (result == VM_COMPILE_ERROR) {
            printf("Compile time error.\n");
        } 
        else if (result == VM_RUNTIME_ERROR) {
            printf("Runtime error.\n");
        }
    }

    return 0;
}