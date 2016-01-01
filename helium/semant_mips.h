#ifndef SEMANT_MIPS_H_ICPDBCH2
#define SEMANT_MIPS_H_ICPDBCH2

#include "program.h"
#include "ast_asm.h"

typedef struct SemantMIPS_Context_t
{
    Program_Module module;
    size_t errors;

} * SemantMIPS_Context;

int SemantMIPS_Translate (Program_Module m, A_asmStmList l);

#endif /* end of include guard: SEMANT_MIPS_H_ICPDBCH2 */
