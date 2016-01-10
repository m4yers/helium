#ifndef SEMANT_MIPS_H_ICPDBCH2
#define SEMANT_MIPS_H_ICPDBCH2

#include "program.h"
#include "semant.h"
#include "ast_asm.h"

typedef struct Sema_MIPSContext_t
{
    struct A_asmDec_t * dec;
    Sema_Context context;
    Program_Module module;
    size_t errors;

} * SemantMIPS_Context;

int SemantMIPS_Translate (Sema_Context c, struct A_asmDec_t * d);

#endif /* end of include guard: SEMANT_MIPS_H_ICPDBCH2 */
