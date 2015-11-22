#include "mipsmachine.h"

const int F_wordSize = 4;

void MIPS_Init()
{
    zero = Temp_NewTemp();

    at = Temp_NewTemp();

    v0 = Temp_NewTemp();
    v1 = Temp_NewTemp();

    a0 = Temp_NewTemp();
    a1 = Temp_NewTemp();
    a2 = Temp_NewTemp();
    a3 = Temp_NewTemp();

    t0 = Temp_NewTemp();
    t1 = Temp_NewTemp();
    t2 = Temp_NewTemp();
    t3 = Temp_NewTemp();
    t4 = Temp_NewTemp();
    t5 = Temp_NewTemp();
    t6 = Temp_NewTemp();
    t7 = Temp_NewTemp();
    t8 = Temp_NewTemp();
    t9 = Temp_NewTemp();

    s0 = Temp_NewTemp();
    s1 = Temp_NewTemp();
    s2 = Temp_NewTemp();
    s3 = Temp_NewTemp();
    s4 = Temp_NewTemp();
    s5 = Temp_NewTemp();
    s6 = Temp_NewTemp();
    s7 = Temp_NewTemp();

    k0 = Temp_NewTemp();
    k1 = Temp_NewTemp();

    gp = Temp_NewTemp();

    sp = Temp_NewTemp();
    fp = Temp_NewTemp();
    ra = Temp_NewTemp();

    regs_all = F_Registers (NULL, NULL);
    F_RegistersAdd (regs_all, zero, "$zero");

    F_RegistersAdd (regs_all, at, "$at");

    F_RegistersAdd (regs_all, v0, "$v0");
    F_RegistersAdd (regs_all, v1, "$v1");

    F_RegistersAdd (regs_all, a0, "$a0");
    F_RegistersAdd (regs_all, a1, "$a1");
    F_RegistersAdd (regs_all, a2, "$a2");
    F_RegistersAdd (regs_all, a3, "$a3");

    F_RegistersAdd (regs_all, t0, "$t0");
    F_RegistersAdd (regs_all, t1, "$t1");
    F_RegistersAdd (regs_all, t2, "$t2");
    F_RegistersAdd (regs_all, t3, "$t3");
    F_RegistersAdd (regs_all, t4, "$t4");
    F_RegistersAdd (regs_all, t5, "$t5");
    F_RegistersAdd (regs_all, t6, "$t6");
    F_RegistersAdd (regs_all, t7, "$t7");
    F_RegistersAdd (regs_all, t8, "$t8");
    F_RegistersAdd (regs_all, t9, "$t9");

    F_RegistersAdd (regs_all, s0, "$s0");
    F_RegistersAdd (regs_all, s1, "$s1");
    F_RegistersAdd (regs_all, s2, "$s2");
    F_RegistersAdd (regs_all, s3, "$s3");
    F_RegistersAdd (regs_all, s4, "$s4");
    F_RegistersAdd (regs_all, s5, "$s5");
    F_RegistersAdd (regs_all, s6, "$s6");
    F_RegistersAdd (regs_all, s7, "$s7");

    F_RegistersAdd (regs_all, k0, "$k0");
    F_RegistersAdd (regs_all, k1, "$k1");

    F_RegistersAdd (regs_all, gp, "$gp");

    F_RegistersAdd (regs_all, sp, "$sp");
    F_RegistersAdd (regs_all, fp, "$fp");
    F_RegistersAdd (regs_all, ra, "$ra");

    regs_colors = F_Registers (NULL, NULL);

    F_RegistersAdd (regs_colors, v0, "$v0");
    /* F_RegistersAdd (regs_colors, v1, "$v1"); */

    F_RegistersAdd (regs_colors, a0, "$a0");
    F_RegistersAdd (regs_colors, a1, "$a1");
    F_RegistersAdd (regs_colors, a2, "$a2");
    F_RegistersAdd (regs_colors, a3, "$a3");

    F_RegistersAdd (regs_colors, t0, "$t0");
    F_RegistersAdd (regs_colors, t1, "$t1");
    F_RegistersAdd (regs_colors, t2, "$t2");
    F_RegistersAdd (regs_colors, t3, "$t3");
    F_RegistersAdd (regs_colors, t4, "$t4");
    F_RegistersAdd (regs_colors, t5, "$t5");
    F_RegistersAdd (regs_colors, t6, "$t6");
    F_RegistersAdd (regs_colors, t7, "$t7");
    F_RegistersAdd (regs_colors, t8, "$t8");
    F_RegistersAdd (regs_colors, t9, "$t9");

    F_RegistersAdd (regs_colors, s0, "$s0");
    F_RegistersAdd (regs_colors, s1, "$s1");
    F_RegistersAdd (regs_colors, s2, "$s2");
    F_RegistersAdd (regs_colors, s3, "$s3");
    F_RegistersAdd (regs_colors, s4, "$s4");
    F_RegistersAdd (regs_colors, s5, "$s5");
    F_RegistersAdd (regs_colors, s6, "$s6");
    F_RegistersAdd (regs_colors, s7, "$s7");

    /* F_RegistersAdd (regs_colors, sp, "$sp"); */
    /* F_RegistersAdd (regs_colors, fp, "$fp"); */
    F_RegistersAdd (regs_colors, ra, "$ra");

    regs_result = F_Registers (NULL, NULL);
    F_RegistersAdd(regs_result, v0, "$v0");
    F_RegistersAdd(regs_result, v1, "$v1");

    regs_special = F_Registers (NULL, NULL);
    F_RegistersAdd (regs_special, zero, "zero");
    F_RegistersAdd (regs_special, sp,   "$sp");
    F_RegistersAdd (regs_special, fp,   "$fp");
    F_RegistersAdd (regs_special, ra,   "$ra");
    /* F_RegistersAdd (regs_special, v0,   "$v0"); */
    /* F_RegistersAdd (regs_special, v1,   "$v1"); */

    regs_arguments = F_Registers (NULL, NULL);
    F_RegistersAdd (regs_arguments, a0, "$a0");
    F_RegistersAdd (regs_arguments, a1, "$a1");
    F_RegistersAdd (regs_arguments, a2, "$a2");
    F_RegistersAdd (regs_arguments, a3, "$a3");

    regs_caller_save = F_Registers (NULL, NULL);
    F_RegistersAdd (regs_caller_save, t0, "$t0");
    F_RegistersAdd (regs_caller_save, t1, "$t1");
    F_RegistersAdd (regs_caller_save, t2, "$t2");
    F_RegistersAdd (regs_caller_save, t3, "$t3");
    F_RegistersAdd (regs_caller_save, t4, "$t4");
    F_RegistersAdd (regs_caller_save, t5, "$t5");
    F_RegistersAdd (regs_caller_save, t6, "$t6");
    F_RegistersAdd (regs_caller_save, t7, "$t7");
    F_RegistersAdd (regs_caller_save, t8, "$t8");
    F_RegistersAdd (regs_caller_save, t9, "$t9");

    regs_callee_save = F_Registers (NULL, NULL);
    // fp is always used by the callee
    F_RegistersAdd (regs_callee_save, fp, "$fp");
    F_RegistersAdd (regs_callee_save, s7, "$s7");
    F_RegistersAdd (regs_callee_save, s6, "$s6");
    F_RegistersAdd (regs_callee_save, s5, "$s5");
    F_RegistersAdd (regs_callee_save, s4, "$s4");
    F_RegistersAdd (regs_callee_save, s3, "$s3");
    F_RegistersAdd (regs_callee_save, s2, "$s2");
    F_RegistersAdd (regs_callee_save, s1, "$s1");
    F_RegistersAdd (regs_callee_save, s0, "$s0");
}
