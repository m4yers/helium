#ifndef SEMANT_H_VYAZ1M7T
#define SEMANT_H_VYAZ1M7T

#include "translate.h"
#include "program.h"

#include "ext/stack.h"

typedef struct Semant_ContextType
{
    Program_Module module;
    Tr_level global;
    Tr_level level;
    S_table venv;
    S_table tenv;
    Temp_label breaker;
    int loopNesting;

} * Semant_Context;

int Semant_Translate (Program_Module m);

#endif /* end of include guard: SEMANT_H_VYAZ1M7T */
