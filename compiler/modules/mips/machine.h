#ifndef MIPSMACHINE_H_LFTURR6G
#define MIPSMACHINE_H_LFTURR6G

#include "core/machine.h"

/*
 * Defines for MIPS ISA format, taken from GNU binutils
 *
 * These are the characters which may appears in the format field of an instruction. They appear in
 * the order in which the fields appear when the instruction is used.
 */

/** general */
#define SHIFT_AMOUNT_5_BIT                                     '<'
#define SHIFT_AMOUNT_32_63                                     '>'
#define TARGET_ADDRESS_26_BIT                                  'a'
#define BASE_REGISTER_5_BIT                                    'b'
#define BREAKPOINT_CODE_10_BIT                                 'c'
#define DESTINATION_REGISTER_5_BIT                             'd'
#define PREFIX_HINT_5_BIT                                      'h'
#define UNSIGNED_IMMEDIATE_16_BIT                              'i'
#define SIGNED_IMMEDIATE_16_BIT                                'j'
#define CACHE_OPCODE_IN_TARGET_REGISTER_POS_5_BIT              'k'
#define SIGNED_OFFSET_16_BIT                                   'o'
#define PC_RELATIVE_BRANCH_TARGET_16_BIT                       'p'
#define SAME_REGISTER_SOURCE_AND_TARGET_5_BIT                  'r'
#define SOURCE_REGISTER_5_BIT                                  's'
#define TARGET_REGISTER_5_BIT                                  't'
#define UPPER_16_BITS_OF_ADDRESS_16_BIT                        'u'
#define SAME_REGISTER_SOURCE_AND_DESTINATION_5_BIT             'v'
#define SAME_REGISTER_TARGET_AND_DESTINATION_5_BIT             'w'
#define COPROCESSOR_FUNCTION_CODE_25_BIT                       'C'
#define SYSCALL_FUNCTION_CODE_20_BIT                           'B'
#define ACCEPT_AND_IGNORE_REGISTER_NAME                        'x'
#define ZERO_REGISTER                                          'z'

/** floating point instructions */
#define FLOAT_DESTINATION_REGISTER_5_BIT                       'D'
#define FLOAT_COMPARE_CONDITION_CODE_3_BIT                     'M'
#define FLOAT_BRANCH_CONDITION_CODE_3_BIT                      'N'
#define FLOAT_FS_SOURCE_1_REGISTER_5_BIT                       'S'
#define FLOAT_FT_SOURCE_2_REGISTER_5_BIT                       'T'
#define FLOAT_FR_SOURCE_3_REGISTER_5_BIT                       'R'
#define FLOAT_SAME_REGISTER_SOURCE_AND_DESTINATION_5_BIT       'V'
#define FLOAT_SAME_REGISTER_TARGET_AND_DESTINATION_5_BIT       'W'

/** coprocessor */
#define COP_TARGET_REGISTER_5_BIT                              'E'
#define COP_DESTINATON_REGISTER_5_BIT                          'G'

/** macro */
#define MA_LITERAL_ADDRESS                                     'P'
#define MA_GENERAL_EXPRESSION_32_BIT                           'A'
#define MA_IMMEDIATE_32_BIT                                    'I'
#define MA_FLOAT_CONSTANT_RDATA_64_BIT                         'F'
#define MA_FLOAT_CONSTANT_LIT8_64_BIT                          'L'
#define MA_FLOAT_CONSTANT_32_BIT                               'f'
#define MA_FLOAT_CONSTANT_LIT8_32_BIT                          'l'

/*
 * List of all MIPS opcodes including format
 */
extern const struct M_opCode_t mips_opcodes[];
extern const size_t mips_opcodes_num;

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

Temp_map regs_map;

M_regs regs_all;
M_regs regs_colors;
M_regs regs_result;
M_regs regs_special;
M_regs regs_arguments;
M_regs regs_callee_save;
M_regs regs_caller_save;

void MIPS_Init (void);

#endif /* end of include guard: MIPSMACHINE_H_LFTURR6G */
