#ifndef SEMANT_H_VYAZ1M7T
#define SEMANT_H_VYAZ1M7T

#include "translate.h"
#include "program.h"

#include "ext/stack.h"

typedef struct Semant_Context_t
{
    Program_Module module;
    Tr_level global;
    Tr_level level;
    S_table venv;
    S_table tenv;
    Temp_label breaker;
    int loopNesting;

} * Semant_Context;

typedef struct Semant_Exp_t
{
    Tr_exp exp;
    Ty_ty ty;

} Semant_Exp;


int Semant_Translate (Program_Module m);

Semant_Exp TransScope (Semant_Context context, A_scope scope);
Ty_ty      TransTyp (Semant_Context context, A_ty ty);
Tr_exp     TransDec (Semant_Context context, A_dec dec);
Semant_Exp TransExp (Semant_Context context, A_exp exp);
Semant_Exp TransVar (Semant_Context context, A_var var, bool deref);

#endif /* end of include guard: SEMANT_H_VYAZ1M7T */
