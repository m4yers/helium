#ifndef SEMANT_MIPS_H_ICPDBCH2
#define SEMANT_MIPS_H_ICPDBCH2

#include "util/table.h"
#include "core/program.h"

#include "modules/helium/semant.h"
#include "modules/mips/ir.h"

typedef struct Sema_mipsContext_t
{
    struct A_asmDec_t * dec;
    Sema_Context context;
    Program_Module module;
    struct Vector_t /* Temp_label */ labels;
    TAB_table /* S_symbol: Temp_temp  */ meta_regs;
    TAB_table /* S_symbol: Temp_label */ meta_labs;
    TAB_table /* S_symbol: Temp_label */ norm_labs;
    size_t errors;

} * Sema_mipsContext;

IR_mipsStmList SemantMIPS_Translate (Sema_Context cntx, struct A_asmDec_t * dec);

#endif /* end of include guard: SEMANT_MIPS_H_ICPDBCH2 */
