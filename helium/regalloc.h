#ifndef REGALLOC_H_6JKE4GXK
#define REGALLOC_H_6JKE4GXK

#include "asm.h"
#include "frame.h"
#include "machine.h"

typedef struct RA_ResultType
{
    Temp_tempList colors;
    Temp_map coloring;
    ASM_lineList il;

} * RA_Result;

RA_Result RA_RegAlloc (F_frame f, ASM_lineList ll, F_registers regs_all, F_registers regs_colors);

#endif /* end of include guard: REGALLOC_H_6JKE4GXK */
