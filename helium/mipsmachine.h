#ifndef MIPSMACHINE_H_LFTURR6G
#define MIPSMACHINE_H_LFTURR6G

#include "machine.h"

const struct M_opCode_t MIPS_opCodes[] =
{
    M_OpCode ("abs", M_opCode_I),
    M_OpCode ("add", M_opCode_I),
    M_OpCode ("addciu", M_opCode_I),
    M_OpCode ("addu", M_opCode_I),
    M_OpCode ("and", M_opCode_I),
    M_OpCode ("b", M_opCode_I),
    M_OpCode ("bal", M_opCode_I),
    M_OpCode ("bc0f", M_opCode_I),
    M_OpCode ("bc0fl", M_opCode_I),
    M_OpCode ("bc0t", M_opCode_I),
    M_OpCode ("bc0tlbc1f", M_opCode_I),
    M_OpCode ("bc1fl", M_opCode_I),
    M_OpCode ("bc1t", M_opCode_I),
    M_OpCode ("bc1tl", M_opCode_I),
    M_OpCode ("bc2f", M_opCode_I),
    M_OpCode ("bc2fl", M_opCode_I),
    M_OpCode ("bc2t", M_opCode_I),
    M_OpCode ("bc2tl", M_opCode_I),
    M_OpCode ("beq", M_opCode_I),
    M_OpCode ("beql", M_opCode_I),
    M_OpCode ("beqz", M_opCode_I),
    M_OpCode ("beqzl", M_opCode_I),
    M_OpCode ("bge", M_opCode_I),
    M_OpCode ("bgel", M_opCode_I),
    M_OpCode ("bgeu", M_opCode_I),
    M_OpCode ("bgeul", M_opCode_I),
    M_OpCode ("bgez", M_opCode_I),
    M_OpCode ("bgezal", M_opCode_I),
    M_OpCode ("bgezall", M_opCode_I),
    M_OpCode ("bgezl", M_opCode_I),
    M_OpCode ("bgt", M_opCode_I),
    M_OpCode ("bgtl", M_opCode_I),
    M_OpCode ("bgtu", M_opCode_I),
    M_OpCode ("bgtul", M_opCode_I),
    M_OpCode ("bgtz", M_opCode_I),
    M_OpCode ("bgtzl", M_opCode_I),
    M_OpCode ("ble", M_opCode_I),
    M_OpCode ("blel", M_opCode_I),
    M_OpCode ("bleu", M_opCode_I),
    M_OpCode ("bleul", M_opCode_I),
    M_OpCode ("blez", M_opCode_I),
    M_OpCode ("blezl", M_opCode_I),
    M_OpCode ("blt", M_opCode_I),
    M_OpCode ("bltl", M_opCode_I),
    M_OpCode ("bltu", M_opCode_I),
    M_OpCode ("bltul", M_opCode_I),
    M_OpCode ("bltz", M_opCode_I),
    M_OpCode ("bltzal", M_opCode_I),
    M_OpCode ("bltzall", M_opCode_I),
    M_OpCode ("bltzl", M_opCode_I),
    M_OpCode ("bne", M_opCode_I),
    M_OpCode ("bnel", M_opCode_I),
    M_OpCode ("bnez", M_opCode_I),
    M_OpCode ("bnezl", M_opCode_I),
    M_OpCode ("break", M_opCode_I),
    M_OpCode ("cache", M_opCode_I),
    M_OpCode ("cfc0", M_opCode_I),
    M_OpCode ("cfc1", M_opCode_I),
    M_OpCode ("cfc2", M_opCode_I),
    M_OpCode ("ctc0", M_opCode_I),
    M_OpCode ("ctc1", M_opCode_I),
    M_OpCode ("ctc2", M_opCode_I),
    M_OpCode ("dabs", M_opCode_I),
    M_OpCode ("dadd", M_opCode_I),
    M_OpCode ("daddi", M_opCode_I),
    M_OpCode ("daddiu", M_opCode_I),
    M_OpCode ("daddu", M_opCode_I),
    M_OpCode ("ddiv", M_opCode_I),
    M_OpCode ("ddivd", M_opCode_I),
    M_OpCode ("ddivdu", M_opCode_I),
    M_OpCode ("ddivu", M_opCode_I),
    M_OpCode ("div", M_opCode_I),
    M_OpCode ("divd", M_opCode_I),
    M_OpCode ("divdu", M_opCode_I),
    M_OpCode ("divo", M_opCode_I),
    M_OpCode ("divou", M_opCode_I),
    M_OpCode ("divu", M_opCode_I),
    M_OpCode ("dla", M_opCode_I),
    M_OpCode ("dli", M_opCode_I),
    M_OpCode ("dmadd16", M_opCode_I),
    M_OpCode ("dmfc0", M_opCode_I),
    M_OpCode ("dmfc1", M_opCode_I),
    M_OpCode ("dmfc2", M_opCode_I),
    M_OpCode ("dmtc0", M_opCode_I),
    M_OpCode ("dmtc1", M_opCode_I),
    M_OpCode ("dmtc2", M_opCode_I),
    M_OpCode ("dmul", M_opCode_I),
    M_OpCode ("dmulo", M_opCode_I),
    M_OpCode ("dmulou", M_opCode_I),
    M_OpCode ("dmult", M_opCode_I),
    M_OpCode ("dmultu", M_opCode_I),
    M_OpCode ("dneg", M_opCode_I),
    M_OpCode ("dnegu", M_opCode_I),
    M_OpCode ("drem", M_opCode_I),
    M_OpCode ("dremu", M_opCode_I),
    M_OpCode ("dret", M_opCode_I),
    M_OpCode ("drol", M_opCode_I),
    M_OpCode ("dror", M_opCode_I),
    M_OpCode ("dsll", M_opCode_I),
    M_OpCode ("dsllv", M_opCode_I),
    M_OpCode ("dsll32", M_opCode_I),
    M_OpCode ("dsra", M_opCode_I),
    M_OpCode ("dsra32", M_opCode_I),
    M_OpCode ("dsrl", M_opCode_I),
    M_OpCode ("dsrlv", M_opCode_I),
    M_OpCode ("dsrl32", M_opCode_I),
    M_OpCode ("dsub", M_opCode_I),
    M_OpCode ("dsubu", M_opCode_I),
    M_OpCode ("eret", M_opCode_I),
    M_OpCode ("ffc", M_opCode_I),
    M_OpCode ("ffs", M_opCode_I),
    M_OpCode ("flushd", M_opCode_I) ,
    M_OpCode ("j", M_opCode_I) ,
    M_OpCode ("jr", M_opCode_I) ,
    M_OpCode ("jal", M_opCode_I) ,
    M_OpCode ("jalr", M_opCode_I) ,
    M_OpCode ("la", M_opCode_I) ,
    M_OpCode ("lb", M_opCode_I) ,
    M_OpCode ("lbu", M_opCode_I) ,
    M_OpCode ("ld", M_opCode_I) ,
    M_OpCode ("ldl", M_opCode_I),
    M_OpCode ("ldr", M_opCode_I) ,
    M_OpCode ("ldxc1", M_opCode_I) ,
    M_OpCode ("lh", M_opCode_I),
    M_OpCode ("lhu", M_opCode_I),
    M_OpCode ("li", M_opCode_I),
    M_OpCode ("ll", M_opCode_I),
    M_OpCode ("lld", M_opCode_I),
    M_OpCode ("lui", M_opCode_I),
    M_OpCode ("lw", M_opCode_I),
    M_OpCode ("lwc1", M_opCode_I),
    M_OpCode ("lwl", M_opCode_I),
    M_OpCode ("lwr", M_opCode_I),
    M_OpCode ("lwu", M_opCode_I),
    M_OpCode ("lwxc1", M_opCode_I),
    M_OpCode ("madd", M_opCode_I),
    M_OpCode ("maddu", M_opCode_I),
    M_OpCode ("mad", M_opCode_I),
    M_OpCode ("madu", M_opCode_I),
    M_OpCode ("madd16", M_opCode_I),
    M_OpCode ("max", M_opCode_I),
    M_OpCode ("mfc0", M_opCode_I),
    M_OpCode ("mfc1", M_opCode_I),
    M_OpCode ("mfc2", M_opCode_I),
    M_OpCode ("mfhi", M_opCode_I),
    M_OpCode ("mflo", M_opCode_I),
    M_OpCode ("min", M_opCode_I),
    M_OpCode ("move", M_opCode_I),
    M_OpCode ("movf", M_opCode_I),
    M_OpCode ("movn", M_opCode_I),
    M_OpCode ("movt", M_opCode_I),
    M_OpCode ("movz", M_opCode_I),
    M_OpCode ("msub", M_opCode_I),
    M_OpCode ("msubu", M_opCode_I),
    M_OpCode ("mtc0", M_opCode_I),
    M_OpCode ("mtc1", M_opCode_I),
    M_OpCode ("mtc2", M_opCode_I),
    M_OpCode ("mthi", M_opCode_I),
    M_OpCode ("mtlo", M_opCode_I),
    M_OpCode ("mul", M_opCode_I),
    M_OpCode ("mulu", M_opCode_I),
    M_OpCode ("mulo", M_opCode_I),
    M_OpCode ("mulou", M_opCode_I),
    M_OpCode ("mult", M_opCode_I),
    M_OpCode ("multu", M_opCode_I),
    M_OpCode ("neg", M_opCode_I),
    M_OpCode ("negu", M_opCode_I),
    M_OpCode ("nop", M_opCode_I),
    M_OpCode ("nor", M_opCode_I),
    M_OpCode ("not", M_opCode_I),
    M_OpCode ("or", M_opCode_I),
    M_OpCode ("ori", M_opCode_I),
    M_OpCode ("pref", M_opCode_I),
    M_OpCode ("prefx", M_opCode_I),
    M_OpCode ("r2u", M_opCode_I),
    M_OpCode ("radd", M_opCode_I),
    M_OpCode ("rem", M_opCode_I),
    M_OpCode ("remu", M_opCode_I),
    M_OpCode ("rfe", M_opCode_I),
    M_OpCode ("rmul", M_opCode_I),
    M_OpCode ("rol", M_opCode_I),
    M_OpCode ("ror", M_opCode_I),
    M_OpCode ("rsub", M_opCode_I),
    M_OpCode ("sb", M_opCode_I),
    M_OpCode ("sc", M_opCode_I),
    M_OpCode ("scd", M_opCode_I),
    M_OpCode ("sd", M_opCode_I),
    M_OpCode ("sdbbp", M_opCode_I),
    M_OpCode ("sdc1", M_opCode_I),
    M_OpCode ("sdl", M_opCode_I),
    M_OpCode ("sdr", M_opCode_I),
    M_OpCode ("sdxc1", M_opCode_I),
    M_OpCode ("selsl", M_opCode_I),
    M_OpCode ("selsr", M_opCode_I),
    M_OpCode ("seq", M_opCode_I),
    M_OpCode ("sge", M_opCode_I),
    M_OpCode ("sgeu", M_opCode_I),
    M_OpCode ("sgt", M_opCode_I),
    M_OpCode ("sgtu", M_opCode_I),
    M_OpCode ("sh", M_opCode_I),
    M_OpCode ("sle", M_opCode_I),
    M_OpCode ("sleu", M_opCode_I),
    M_OpCode ("sll", M_opCode_I),
    M_OpCode ("sllv", M_opCode_I),
    M_OpCode ("slt", M_opCode_I),
    M_OpCode ("slti", M_opCode_I),
    M_OpCode ("sltiu", M_opCode_I),
    M_OpCode ("sltu", M_opCode_I),
    M_OpCode ("sne", M_opCode_I),
    M_OpCode ("sra", M_opCode_I),
    M_OpCode ("srav", M_opCode_I),
    M_OpCode ("srl", M_opCode_I),
    M_OpCode ("srlv", M_opCode_I),
    M_OpCode ("standby", M_opCode_I),
    M_OpCode ("sub", M_opCode_I),
    M_OpCode ("subu", M_opCode_I),
    M_OpCode ("suspend", M_opCode_I),
    M_OpCode ("sw", M_opCode_I),
    M_OpCode ("swc1", M_opCode_I),
    M_OpCode ("swl", M_opCode_I),
    M_OpCode ("swr", M_opCode_I),
    M_OpCode ("swxc1", M_opCode_I),
    M_OpCode ("sync", M_opCode_I),
    M_OpCode ("syscall", M_opCode_I),
    M_OpCode ("teq", M_opCode_I),
    M_OpCode ("teqi", M_opCode_I),
    M_OpCode ("tge", M_opCode_I),
    M_OpCode ("tgei", M_opCode_I),
    M_OpCode ("tgeiu", M_opCode_I),
    M_OpCode ("tgeu", M_opCode_I),
    M_OpCode ("tlbp", M_opCode_I),
    M_OpCode ("tlbr", M_opCode_I),
    M_OpCode ("tlbwi", M_opCode_I),
    M_OpCode ("tlbwr", M_opCode_I),
    M_OpCode ("tlt", M_opCode_I),
    M_OpCode ("tlti", M_opCode_I),
    M_OpCode ("tltiu", M_opCode_I),
    M_OpCode ("tltu", M_opCode_I),
    M_OpCode ("tne", M_opCode_I),
    M_OpCode ("tnei", M_opCode_I),
    M_OpCode ("u2r", M_opCode_I),
    M_OpCode ("uld", M_opCode_I),
    M_OpCode ("ulh", M_opCode_I),
    M_OpCode ("ulhu", M_opCode_I),
    M_OpCode ("ulw", M_opCode_I),
    M_OpCode ("usd", M_opCode_I),
    M_OpCode ("ushusw", M_opCode_I),
    M_OpCode ("waiti", M_opCode_I),
    M_OpCode ("wb", M_opCode_I),
    M_OpCode ("xor", M_opCode_I),
    M_OpCode ("xori", M_opCode_I),
    M_OpCode ("abs.s", M_opCode_I),
    M_OpCode ("add.s", M_opCode_I),
    M_OpCode ("c.eq.s", M_opCode_I),
    M_OpCode ("c.f.s", M_opCode_I),
    M_OpCode ("c.le.s", M_opCode_I),
    M_OpCode ("c.lt.s", M_opCode_I),
    M_OpCode ("c.nge.s", M_opCode_I),
    M_OpCode ("c.ngl.s", M_opCode_I),
    M_OpCode ("c.ngt.s", M_opCode_I),
    M_OpCode ("c.ole.s", M_opCode_I),
    M_OpCode ("c.olt.s", M_opCode_I),
    M_OpCode ("c.seq.s", M_opCode_I),
    M_OpCode ("c.sf.s", M_opCode_I),
    M_OpCode ("c.ueq.s", M_opCode_I),
    M_OpCode ("c.ule.s", M_opCode_I),
    M_OpCode ("c.ult.s", M_opCode_I),
    M_OpCode ("c.un.s", M_opCode_I),
    M_OpCode ("ceil.l.d", M_opCode_I),
    M_OpCode ("ceil.l.s", M_opCode_I),
    M_OpCode ("ceil.w.d", M_opCode_I),
    M_OpCode ("ceil.w.s", M_opCode_I),
    M_OpCode ("cvt.d.l", M_opCode_I),
    M_OpCode ("cvt.d.s", M_opCode_I),
    M_OpCode ("cvt.d.w", M_opCode_I),
    M_OpCode ("cvt.l.d", M_opCode_I),
    M_OpCode ("cvt.l.s", M_opCode_I),
    M_OpCode ("cvt.s.d", M_opCode_I),
    M_OpCode ("cvt.s.l", M_opCode_I),
    M_OpCode ("cvt.s.w", M_opCode_I),
    M_OpCode ("cvt.w.d", M_opCode_I),
    M_OpCode ("cvt.w.s", M_opCode_I),
    M_OpCode ("div.s", M_opCode_I),
    M_OpCode ("floor.l.d", M_opCode_I),
    M_OpCode ("floor.l.s", M_opCode_I),
    M_OpCode ("floor.w.d", M_opCode_I),
    M_OpCode ("floor.w.s", M_opCode_I),
    M_OpCode ("l.d", M_opCode_I),
    M_OpCode ("l.s", M_opCode_I),
    M_OpCode ("ldc1", M_opCode_I),
    M_OpCode ("madd.s", M_opCode_I),
    M_OpCode ("mov.s", M_opCode_I),
    M_OpCode ("movf.s", M_opCode_I),
    M_OpCode ("movn.s", M_opCode_I),
    M_OpCode ("movt.s", M_opCode_I),
    M_OpCode ("movz.s", M_opCode_I),
    M_OpCode ("msub.s", M_opCode_I),
    M_OpCode ("mul.s", M_opCode_I),
    M_OpCode ("neg.s", M_opCode_I),
    M_OpCode ("nmadd.s", M_opCode_I),
    M_OpCode ("nmsub.s", M_opCode_I),
    M_OpCode ("recip.s", M_opCode_I),
    M_OpCode ("round.l.d", M_opCode_I),
    M_OpCode ("round.l.s", M_opCode_I),
    M_OpCode ("round.w.d", M_opCode_I),
    M_OpCode ("round.w.s", M_opCode_I),
    M_OpCode ("rsqrt.s", M_opCode_I),
    M_OpCode ("s.d", M_opCode_I),
    M_OpCode ("s.s", M_opCode_I),
    M_OpCode ("sqrt.s", M_opCode_I),
    M_OpCode ("sub.s", M_opCode_I),
    M_OpCode ("trunc.l.d", M_opCode_I),
    M_OpCode ("trunc.l.s", M_opCode_I)

};

/**
 * Always has the value 0. Any writes to this register are ignored.
 */
Temp_temp zero;

/**
 * Assembler temporary.
 */
Temp_temp at;

/**
 * Function result registers.
 *
 * Functions return integer results in v0, and 64-bit integer results
 * in v0 and v1 when using 32-bit registers.
 *
 * In cases where floating-point hardware is not present, or when compiler
 * options enable floating-point emulation, functions return single precision
 * floating-point results in v0 and double precision floating-point results
 * in v0 and v1 when using 32-bit registers.
 *
 * v0 and v1 can be temporary registers.
 *
 * Not preserved across function calls.
 */
Temp_temp v0, v1;

/**
 * Function argument registers that hold the first four words of integer
 * type arguments.
 *
 * Functions use these registers to hold floating-point
 * arguments.
 *
 * When floating-point hardware is not present, or compiler options enable
 * floating-point emulation, functions use a0 to hold the first single
 * precision floating-point argument and a1 to hold the second single
 * precision floating-point argument.
 *
 * Functions use a0-a1 for the first double precision floating-point
 * argument, and a2-a3 to hold the second double precision floating-point
 * argument.
 *
 * Not preserved across function calls.
 */
Temp_temp a0, a1, a2, a3;

/**
 * Temporary registers you can use as you want.
 *
 * Not preserved across function calls.
 */
Temp_temp t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;

/**
 * Saved registers to use freely.
 *
 * Preserved across function calls. These registers must be saved before
 * use by the called function.
 */
Temp_temp s0, s1, s2, s3, s4, s5, s6, s7;

/**
 * Reserved for use by the operating system kernel and for exception return.
 */
Temp_temp k0, k1;

/**
 * Global pointer.
 *
 * Kernel implementation dependent.
 */
Temp_temp gp;

/**
 * Stack pointer.
 */
Temp_temp sp;

/**
 * Frame pointer
 */
Temp_temp fp;

/**
 * Return address register, saved by the calling function. Available for
 * use after saving.
 */
Temp_temp ra;

F_registers regs_all;
F_registers regs_colors;
F_registers regs_result;
F_registers regs_special;
F_registers regs_arguments;
F_registers regs_callee_save;
F_registers regs_caller_save;

void MIPS_Init (void);

#endif /* end of include guard: MIPSMACHINE_H_LFTURR6G */
