#ifndef SEMANT_MIPS_H_ICPDBCH2
#define SEMANT_MIPS_H_ICPDBCH2

#include "core/program.h"

#include "modules/helium/semant.h"

typedef struct Sema_MIPSContext_t
{
    struct A_asmDec_t * dec;
    Sema_Context context;
    Program_Module module;
    struct Vector_t /* Temp_label */ labels;
    size_t errors;

} * Sema_MIPSContext;

A_asmStmList SemantMIPS_Translate (Sema_Context c, struct A_asmDec_t * d);

#endif /* end of include guard: SEMANT_MIPS_H_ICPDBCH2 */
