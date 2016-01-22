#ifndef SEMANT_H_VYAZ1M7T
#define SEMANT_H_VYAZ1M7T

#include "util/stack.h"

#include "core/program.h"

#include "modules/helium/translate.h"

typedef struct Sema_Context_t
{
    Program_Module module;
    Tr_level global;
    Tr_level level;
    S_table venv;
    S_table tenv;
    Temp_label breaker;
    int loopNesting;

} * Sema_Context;

typedef struct Sema_Exp_t
{
    Tr_exp exp;
    Ty_ty ty;

} Sema_Exp;


int Semant_Translate (Program_Module m);

Sema_Exp Sema_TransScope (Sema_Context context, A_scope scope);
Ty_ty    Sema_TransTyp (Sema_Context context, A_ty ty);
Tr_exp   Sema_TransDec (Sema_Context context, A_dec dec);
Sema_Exp Sema_TransExp (Sema_Context context, A_exp exp);
Sema_Exp Sema_ValidateVar (Sema_Context context, A_var var);
Sema_Exp Sema_TransVar (Sema_Context context, A_var var, bool deref);

#endif /* end of include guard: SEMANT_H_VYAZ1M7T */
