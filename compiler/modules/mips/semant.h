#ifndef SEMANT_MIPS_H_ICPDBCH2
#define SEMANT_MIPS_H_ICPDBCH2

#include "util/table.h"
#include "core/program.h"

#include "modules/helium/semant.h"

typedef struct Sema_MIPSContext_t
{
    struct A_asmDec_t * dec;
    Sema_Context context;
    Program_Module module;
    struct Vector_t /* Temp_label */ labels;
    TAB_table /* S_symbol: Temp_temp  */ meta_regs;
    TAB_table /* S_symbol: Temp_label */ meta_labs;
    size_t errors;

} * Sema_MIPSContext;

A_asmStmList SemantMIPS_Translate (Sema_Context c, struct A_asmDec_t * d);

#endif /* end of include guard: SEMANT_MIPS_H_ICPDBCH2 */
