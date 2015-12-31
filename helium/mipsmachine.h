#ifndef MIPSMACHINE_H_LFTURR6G
#define MIPSMACHINE_H_LFTURR6G

#include "machine.h"

/*
 * List of all MIPS opcodes including format
 *
 * Format matches the one in binuntils GNU package. Currenlty i do not any 3rd party stuff in the
 * projects so just copy/paste for time being, mb if it grows into something more serious then
 * i better use binutils or whatever provides MIPS ISA in bare code
 *
 * These are the characters which may appears in the format field of an instruction. They appear in
 * the order in which the fields appear when the instruction is used.
 *
 * "a" 26 bit target address
 * "b" 5 bit base register
 * "c" 10 bit breakpoint code
 * "d" 5 bit destination register specifier
 * "h" 5 bit prefx hint
 * "i" 16 bit unsigned immediate
 * "j" 16 bit signed immediate
 * "k" 5 bit cache opcode in target register position
 * "o" 16 bit signed offset
 * "p" 16 bit PC relative branch target address
 * "r" 5 bit same register used as both source and target
 * "s" 5 bit source register specifier
 * "t" 5 bit target register
 * "u" 16 bit upper 16 bits of address
 * "v" 5 bit same register used as both source and destination
 * "w" 5 bit same register used as both target and destination
 * "C" 25 bit coprocessor function code
 * "B" 20 bit syscall function code
 * "x" accept and ignore register name
 * "z" must be zero register
 *
 * Floating point instructions:
 * "D" 5 bit destination register
 * "M" 3 bit compare condition code
 * "N" 3 bit branch condition code
 * "S" 5 bit fs source 1 register
 * "T" 5 bit ft source 2 register
 * "R" 5 bit fr source 3 register
 * "V" 5 bit same register used as floating source and destination
 * "W" 5 bit same register used as floating target and destination
 *
 * Coprocessor instructions:
 * "E" 5 bit target register
 * "G" 5 bit destination register
 *
 * Macro instructions:
 * "A" General 32 bit expression
 * "I" 32 bit immediate
 * "F" 64 bit floating point constant in .rdata
 * "L" 64 bit floating point constant in .lit8
 * "f" 32 bit floating point constant
 * "l" 32 bit floating point constant in .lit4
 */
extern const struct M_opCode_t MIPS_opCodes[];

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
