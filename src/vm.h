#ifndef vm_h
#define vm_h

#include "compiler.h"

#define STACK_MAX 256

typedef struct {
    Bytecode *bytecode;
    int       ip;
    int       stack[STACK_MAX];
    int       stack_top;
} JankyVm;

VmResult run(JankyVm *vm, char *source, int debug);
void     freeVm(JankyVm *vm);

#endif