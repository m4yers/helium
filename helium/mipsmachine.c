#include "mipsmachine.h"

const int F_wordSize = 4;

const struct M_opCode_t mips_opcodes[] =
{
    M_OpCode ("abs",         ""),
    M_OpCode ("nop",          ""),
    M_OpCode ("li",           "t,j"),
    M_OpCode ("li",           "t,i"),
    M_OpCode ("li",           "t,I"),
    M_OpCode ("move",         "d,s"),
    M_OpCode ("move",         "d,s"),
    M_OpCode ("move",         "d,s"),
    M_OpCode ("b",            "p"),
    M_OpCode ("b",            "p"),
    M_OpCode ("bal",          "p"),

    M_OpCode ("abs",          "d,v"),
    M_OpCode ("abs.s",        "D,V"),
    M_OpCode ("abs.d",        "D,V"),
    M_OpCode ("add",          "d,v,t"),
    M_OpCode ("add",          "t,r,I"),
    M_OpCode ("add.s",        "D,V,T"),
    M_OpCode ("add.d",        "D,V,T"),
    M_OpCode ("addi",         "t,r,j"),
    M_OpCode ("addiu",        "t,r,j"),//wtf it should be i(unsigned 16 bit immediate)
    M_OpCode ("addu",         "d,v,t"),
    M_OpCode ("addu",         "t,r,I"),
    M_OpCode ("and",          "d,v,t"),
    M_OpCode ("and",          "t,r,I"),
    M_OpCode ("andi",         "t,r,i"),
    /* b is at the top of the table.  */
    /* bal is at the top of the table.  */
    M_OpCode ("bc0f",         "p"),
    M_OpCode ("bc0fl",        "p"),
    M_OpCode ("bc1f",         "p"),
    M_OpCode ("bc1f",         "N,p"),
    M_OpCode ("bc1fl",        "p"),
    M_OpCode ("bc1fl",        "N,p"),
    M_OpCode ("bc2f",         "p"),
    M_OpCode ("bc2fl",        "p"),
    M_OpCode ("bc3f",         "p"),
    M_OpCode ("bc3fl",        "p"),
    M_OpCode ("bc0t",         "p"),
    M_OpCode ("bc0tl",        "p"),
    M_OpCode ("bc1t",         "p"),
    M_OpCode ("bc1t",         "N,p"),
    M_OpCode ("bc1tl",        "p"),
    M_OpCode ("bc1tl",        "N,p"),
    M_OpCode ("bc2t",         "p"),
    M_OpCode ("bc2tl",        "p"),
    M_OpCode ("bc3t",         "p"),
    M_OpCode ("bc3tl",        "p"),
    M_OpCode ("beqz",         "s,p"),
    M_OpCode ("beqzl",        "s,p"),
    M_OpCode ("beq",          "s,t,p"),
    M_OpCode ("beq",          "s,I,p"),
    M_OpCode ("beql",         "s,t,p"),
    M_OpCode ("beql",         "s,I,p"),
    M_OpCode ("bge",          "s,t,p"),
    M_OpCode ("bge",          "s,I,p"),
    M_OpCode ("bgel",         "s,t,p"),
    M_OpCode ("bgel",         "s,I,p"),
    M_OpCode ("bgeu",         "s,t,p"),
    M_OpCode ("bgeu",         "s,I,p"),
    M_OpCode ("bgeul",        "s,t,p"),
    M_OpCode ("bgeul",        "s,I,p"),
    M_OpCode ("bgez",         "s,p"),
    M_OpCode ("bgezl",        "s,p"),
    M_OpCode ("bgezal",       "s,p"),
    M_OpCode ("bgezall",      "s,p"),
    M_OpCode ("bgt",          "s,t,p"),
    M_OpCode ("bgt",          "s,I,p"),
    M_OpCode ("bgtl",         "s,t,p"),
    M_OpCode ("bgtl",         "s,I,p"),
    M_OpCode ("bgtu",         "s,t,p"),
    M_OpCode ("bgtu",         "s,I,p"),
    M_OpCode ("bgtul",        "s,t,p"),
    M_OpCode ("bgtul",        "s,I,p"),
    M_OpCode ("bgtz",         "s,p"),
    M_OpCode ("bgtzl",        "s,p"),
    M_OpCode ("ble",          "s,t,p"),
    M_OpCode ("ble",          "s,I,p"),
    M_OpCode ("blel",         "s,t,p"),
    M_OpCode ("blel",         "s,I,p"),
    M_OpCode ("bleu",         "s,t,p"),
    M_OpCode ("bleu",         "s,I,p"),
    M_OpCode ("bleul",        "s,t,p"),
    M_OpCode ("bleul",        "s,I,p"),
    M_OpCode ("blez",         "s,p"),
    M_OpCode ("blezl",        "s,p"),
    M_OpCode ("blt",          "s,t,p"),
    M_OpCode ("blt",          "s,I,p"),
    M_OpCode ("bltl",         "s,t,p"),
    M_OpCode ("bltl",         "s,I,p"),
    M_OpCode ("bltu",         "s,t,p"),
    M_OpCode ("bltu",         "s,I,p"),
    M_OpCode ("bltul",        "s,t,p"),
    M_OpCode ("bltul",        "s,I,p"),
    M_OpCode ("bltz",         "s,p"),
    M_OpCode ("bltzl",        "s,p"),
    M_OpCode ("bltzal",       "s,p"),
    M_OpCode ("bltzall",      "s,p"),
    M_OpCode ("bnez",         "s,p"),
    M_OpCode ("bnezl",        "s,p"),
    M_OpCode ("bne",          "s,t,p"),
    M_OpCode ("bne",          "s,I,p"),
    M_OpCode ("bnel",         "s,t,p"),
    M_OpCode ("bnel",         "s,I,p"),
    M_OpCode ("break",        ""),
    M_OpCode ("break",        "c"),
    M_OpCode ("c.f.d",        "S,T"),
    M_OpCode ("c.f.d",        "M,S,T"),
    M_OpCode ("c.f.s",        "S,T"),
    M_OpCode ("c.f.s",        "M,S,T"),
    M_OpCode ("c.un.d",       "S,T"),
    M_OpCode ("c.un.d",       "M,S,T"),
    M_OpCode ("c.un.s",       "S,T"),
    M_OpCode ("c.un.s",       "M,S,T"),
    M_OpCode ("c.eq.d",       "S,T"),
    M_OpCode ("c.eq.d",       "M,S,T"),
    M_OpCode ("c.eq.s",       "S,T"),
    M_OpCode ("c.eq.s",       "M,S,T"),
    M_OpCode ("c.ueq.d",      "S,T"),
    M_OpCode ("c.ueq.d",      "M,S,T"),
    M_OpCode ("c.ueq.s",      "S,T"),
    M_OpCode ("c.ueq.s",      "M,S,T"),
    M_OpCode ("c.olt.d",      "S,T"),
    M_OpCode ("c.olt.d",      "M,S,T"),
    M_OpCode ("c.olt.s",      "S,T"),
    M_OpCode ("c.olt.s",      "M,S,T"),
    M_OpCode ("c.ult.d",      "S,T"),
    M_OpCode ("c.ult.d",      "M,S,T"),
    M_OpCode ("c.ult.s",      "S,T"),
    M_OpCode ("c.ult.s",      "M,S,T"),
    M_OpCode ("c.ole.d",      "S,T"),
    M_OpCode ("c.ole.d",      "M,S,T"),
    M_OpCode ("c.ole.s",      "S,T"),
    M_OpCode ("c.ole.s",      "M,S,T"),
    M_OpCode ("c.ule.d",      "S,T"),
    M_OpCode ("c.ule.d",      "M,S,T"),
    M_OpCode ("c.ule.s",      "S,T"),
    M_OpCode ("c.ule.s",      "M,S,T"),
    M_OpCode ("c.sf.d",       "S,T"),
    M_OpCode ("c.sf.d",       "M,S,T"),
    M_OpCode ("c.sf.s",       "S,T"),
    M_OpCode ("c.sf.s",       "M,S,T"),
    M_OpCode ("c.ngle.d",     "S,T"),
    M_OpCode ("c.ngle.d",     "M,S,T"),
    M_OpCode ("c.ngle.s",     "S,T"),
    M_OpCode ("c.ngle.s",     "M,S,T"),
    M_OpCode ("c.seq.d",      "S,T"),
    M_OpCode ("c.seq.d",      "M,S,T"),
    M_OpCode ("c.seq.s",      "S,T"),
    M_OpCode ("c.seq.s",      "M,S,T"),
    M_OpCode ("c.ngl.d",      "S,T"),
    M_OpCode ("c.ngl.d",      "M,S,T"),
    M_OpCode ("c.ngl.s",      "S,T"),
    M_OpCode ("c.ngl.s",      "M,S,T"),
    M_OpCode ("c.lt.d",       "S,T"),
    M_OpCode ("c.lt.d",       "M,S,T"),
    M_OpCode ("c.lt.s",       "S,T"),
    M_OpCode ("c.lt.s",       "M,S,T"),
    M_OpCode ("c.nge.d",      "S,T"),
    M_OpCode ("c.nge.d",      "M,S,T"),
    M_OpCode ("c.nge.s",      "S,T"),
    M_OpCode ("c.nge.s",      "M,S,T"),
    M_OpCode ("c.le.d",       "S,T"),
    M_OpCode ("c.le.d",       "M,S,T"),
    M_OpCode ("c.le.s",       "S,T"),
    M_OpCode ("c.le.s",       "M,S,T"),
    M_OpCode ("c.ngt.d",      "S,T"),
    M_OpCode ("c.ngt.d",      "M,S,T"),
    M_OpCode ("c.ngt.s",      "S,T"),
    M_OpCode ("c.ngt.s",      "M,S,T"),
    M_OpCode ("cache",        "k,o(b)"),
    M_OpCode ("ceil.l.d",     "D,S"),
    M_OpCode ("ceil.l.s",     "D,S"),
    M_OpCode ("ceil.w.d",     "D,S"),
    M_OpCode ("ceil.w.s",     "D,S"),
    M_OpCode ("cfc0",         "t,G"),
    M_OpCode ("cfc1",         "t,G"),
    M_OpCode ("cfc1",         "t,S"),
    M_OpCode ("cfc2",         "t,G"),
    M_OpCode ("cfc3",         "t,G"),
    M_OpCode ("ctc0",         "t,G"),
    M_OpCode ("ctc1",         "t,G"),
    M_OpCode ("ctc1",         "t,S"),
    M_OpCode ("ctc2",         "t,G"),
    M_OpCode ("ctc3",         "t,G"),
    M_OpCode ("cvt.d.l",      "D,S"),
    M_OpCode ("cvt.d.s",      "D,S"),
    M_OpCode ("cvt.d.w",      "D,S"),
    M_OpCode ("cvt.l.d",      "D,S"),
    M_OpCode ("cvt.l.s",      "D,S"),
    M_OpCode ("cvt.s.l",      "D,S"),
    M_OpCode ("cvt.s.d",      "D,S"),
    M_OpCode ("cvt.s.w",      "D,S"),
    M_OpCode ("cvt.w.d",      "D,S"),
    M_OpCode ("cvt.w.s",      "D,S"),
    M_OpCode ("dabs",         "d,v"),
    M_OpCode ("dadd",         "d,v,t"),
    M_OpCode ("dadd",         "t,r,I"),
    M_OpCode ("daddi",        "t,r,j"),
    M_OpCode ("daddiu",       "t,r,j"),
    M_OpCode ("daddu",        "d,v,t"),
    M_OpCode ("daddu",        "t,r,I"),
    /* For ddiv, see the comments about div.  */
    M_OpCode ("ddiv",         "z,s,t"),
    M_OpCode ("ddiv",         "d,v,t"),
    M_OpCode ("ddiv",         "d,v,I"),
    /* For ddivu, see the comments about div.  */
    M_OpCode ("ddivu",        "z,s,t"),
    M_OpCode ("ddivu",        "d,v,t"),
    M_OpCode ("ddivu",        "d,v,I"),
    /*
     * The MIPS assembler treats the div opcode with two operands as
     * though the first operand appeared twice (the first operand is both
     * a source and a destination).  To get the div machine instruction,
     * you must use an explicit destination of $0.
     */
    M_OpCode ("div",          "z,s,t"),
    M_OpCode ("div",          "d,v,t"),
    M_OpCode ("div",          "d,v,I"),
    M_OpCode ("div.d",        "D,V,T"),
    M_OpCode ("div.s",        "D,V,T"),
    /** For divu, see the comments about div.  */
    M_OpCode ("divu",         "z,s,t"),
    M_OpCode ("divu",         "d,v,t"),
    M_OpCode ("divu",         "d,v,I"),
    M_OpCode ("dla",          "t,A(b)"),
    M_OpCode ("dli",          "t,j"),
    M_OpCode ("dli",          "t,i"),
    M_OpCode ("dli",          "t,I"),
    M_OpCode ("dmadd16",      "s,t"),
    M_OpCode ("dmfc0",        "t,G"),
    M_OpCode ("dmtc0",        "t,G"),
    M_OpCode ("dmfc1",        "t,S"),
    M_OpCode ("dmtc1",        "t,S"),
    M_OpCode ("dmul",         "d,v,t"),
    M_OpCode ("dmul",         "d,v,I"),
    M_OpCode ("dmulo",        "d,v,t"),
    M_OpCode ("dmulo",        "d,v,I"),
    M_OpCode ("dmulou",       "d,v,t"),
    M_OpCode ("dmulou",       "d,v,I"),
    M_OpCode ("dmult",        "s,t"),
    M_OpCode ("dmultu",       "s,t"),
    M_OpCode ("dneg",         "d,w"),
    M_OpCode ("dnegu",        "d,w"),
    M_OpCode ("drem",         "z,s,t"),
    M_OpCode ("drem",         "d,v,t"),
    M_OpCode ("drem",         "d,v,I"),
    M_OpCode ("dremu",        "z,s,t"),
    M_OpCode ("dremu",        "d,v,t"),
    M_OpCode ("dremu",        "d,v,I"),
    M_OpCode ("dsllv",        "d,t,s"),
    M_OpCode ("dsll32",       "d,w,<"),
    M_OpCode ("dsll",         "d,w,s"),
    M_OpCode ("dsll",         "d,w,>"),
    M_OpCode ("dsll",         "d,w,<"),
    M_OpCode ("dsrav",        "d,t,s"),
    M_OpCode ("dsra32",       "d,w,<"),
    M_OpCode ("dsra",         "d,w,s"),
    M_OpCode ("dsra",         "d,w,>"),
    M_OpCode ("dsra",         "d,w,<"),
    M_OpCode ("dsrlv",        "d,t,s"),
    M_OpCode ("dsrl32",       "d,w,<"),
    M_OpCode ("dsrl",         "d,w,s"),
    M_OpCode ("dsrl",         "d,w,>"),
    M_OpCode ("dsrl",         "d,w,<"),
    M_OpCode ("dsub",         "d,v,t"),
    M_OpCode ("dsub",         "d,v,I"),
    M_OpCode ("dsubu",        "d,v,t"),
    M_OpCode ("dsubu",        "d,v,I"),
    M_OpCode ("eret",         ""),
    M_OpCode ("floor.l.d",    "D,S"),
    M_OpCode ("floor.l.s",    "D,S"),
    M_OpCode ("floor.w.d",    "D,S"),
    M_OpCode ("floor.w.s",    "D,S"),
    M_OpCode ("flushi",       ""),
    M_OpCode ("flushd",       ""),
    M_OpCode ("flushid",      ""),
    M_OpCode ("hibernate",    ""),
    M_OpCode ("jr",           "s"),
    M_OpCode ("j",            "s"),
    /*
     * SVR4 PIC code requires special handling for j, so it must be a
     * macro.
     */
    M_OpCode ("j",            "a"),
    /*
     * This form of j is used by the disassembler and internally by the
     * assembler, but will never match user input (because the line above
     * will match first).
     */
    M_OpCode ("j",            "a"),
    M_OpCode ("jalr",         "s"),
    M_OpCode ("jalr",         "d,s"),
    /*
     * SVR4 PIC code requires special handling for jal, so it must be a
     * macro.
     */
    M_OpCode ("jal",          "d,s"),
    M_OpCode ("jal",          "s"),
    M_OpCode ("jal",          "a"),
    /*
     * This form of jal is used by the disassembler and internally by the
     * assembler, but will never match user input (because the line above
     * will match first).
     */
    M_OpCode ("jal",          "a"),
    M_OpCode ("la",           "t,A(b)"),
    M_OpCode ("lb",           "t,o(b)"),
    M_OpCode ("lb",           "t,A(b)"),
    M_OpCode ("lbu",          "t,o(b)"),
    M_OpCode ("lbu",          "t,A(b)"),
    M_OpCode ("ld",           "t,o(b)"),
    M_OpCode ("ld",           "t,o(b)"),
    M_OpCode ("ld",           "t,A(b)"),
    M_OpCode ("ldc1",         "T,o(b)"),
    M_OpCode ("ldc1",         "E,o(b)"),
    M_OpCode ("ldc1",         "T,A(b)"),
    M_OpCode ("ldc1",         "E,A(b)"),
    M_OpCode ("l.d",          "T,o(b)"),
    M_OpCode ("l.d",          "T,o(b)"),
    M_OpCode ("l.d",          "T,A(b)"),
    M_OpCode ("ldc2",         "E,o(b)"),
    M_OpCode ("ldc2",         "E,A(b)"),
    M_OpCode ("ldc3",         "E,o(b)"),
    M_OpCode ("ldc3",         "E,A(b)"),
    M_OpCode ("ldl",          "t,o(b)"),
    M_OpCode ("ldl",          "t,A(b)"),
    M_OpCode ("ldr",          "t,o(b)"),
    M_OpCode ("ldr",          "t,A(b)"),
    M_OpCode ("ldxc1",        "D,t(b)"),
    M_OpCode ("lh",           "t,o(b)"),
    M_OpCode ("lh",           "t,A(b)"),
    M_OpCode ("lhu",          "t,o(b)"),
    M_OpCode ("lhu",          "t,A(b)"),
    /* li is at the start of the table.  */
    M_OpCode ("li.d",         "t,F"),
    M_OpCode ("li.d",         "T,L"),
    M_OpCode ("li.s",         "t,f"),
    M_OpCode ("li.s",         "T,l"),
    M_OpCode ("ll",           "t,o(b)"),
    M_OpCode ("ll",           "t,A(b)"),
    M_OpCode ("lld",          "t,o(b)"),
    M_OpCode ("lld",          "t,A(b)"),
    M_OpCode ("lui",          "t,u"),
    M_OpCode ("lw",           "t,o(b)"),
    M_OpCode ("lw",           "t,A(b)"),
    M_OpCode ("lwc0",         "E,o(b)"),
    M_OpCode ("lwc0",         "E,A(b)"),
    M_OpCode ("lwc1",         "T,o(b)"),
    M_OpCode ("lwc1",         "E,o(b)"),
    M_OpCode ("lwc1",         "T,A(b)"),
    M_OpCode ("lwc1",         "E,A(b)"),
    M_OpCode ("l.s",          "T,o(b)"),
    M_OpCode ("l.s",          "T,A(b)"),
    M_OpCode ("lwc2",         "E,o(b)"),
    M_OpCode ("lwc2",         "E,A(b)"),
    M_OpCode ("lwc3",         "E,o(b)"),
    M_OpCode ("lwc3",         "E,A(b)"),
    M_OpCode ("lwl",          "t,o(b)"),
    M_OpCode ("lwl",          "t,A(b)"),
    M_OpCode ("lcache",       "t,o(b)"),
    M_OpCode ("lcache",       "t,A(b)"),
    M_OpCode ("lwr",          "t,o(b)"),
    M_OpCode ("lwr",          "t,A(b)"),
    M_OpCode ("flush",        "t,o(b)"),
    M_OpCode ("flush",        "t,A(b)"),
    M_OpCode ("lwu",          "t,o(b)"),
    M_OpCode ("lwu",          "t,A(b)"),
    M_OpCode ("lwxc1",        "D,t(b)"),
    M_OpCode ("mad",          "s,t"),
    M_OpCode ("madu",         "s,t"),
    M_OpCode ("addciu",       "t,r,j"),
    M_OpCode ("madd.d",       "D,R,S,T"),
    M_OpCode ("madd.s",       "D,R,S,T"),
    M_OpCode ("madd",         "s,t"),
    M_OpCode ("maddu",        "s,t"),
    M_OpCode ("madd16",       "s,t"),
    M_OpCode ("mfc0",         "t,G"),
    M_OpCode ("mfc1",         "t,S"),
    M_OpCode ("mfc1",         "t,G"),
    M_OpCode ("mfc2",         "t,G"),
    M_OpCode ("mfc3",         "t,G"),
    M_OpCode ("mfhi",         "d"),
    M_OpCode ("mflo",         "d"),
    M_OpCode ("mov.d",        "D,S"),
    M_OpCode ("mov.s",        "D,S"),
    M_OpCode ("movf",         "d,s,N"),
    M_OpCode ("movf.d",       "D,S,N"),
    M_OpCode ("movf.s",       "D,S,N"),
    M_OpCode ("movn",         "d,v,t"),
    M_OpCode ("ffc",          "d,v"),
    M_OpCode ("movn.d",       "D,S,t"),
    M_OpCode ("movn.s",       "D,S,t"),
    M_OpCode ("movt",         "d,s,N"),
    M_OpCode ("movt.d",       "D,S,N"),
    M_OpCode ("movt.s",       "D,S,N"),
    M_OpCode ("movz",         "d,v,t"),
    M_OpCode ("ffs",          "d,v"),
    M_OpCode ("movz.d",       "D,S,t"),
    M_OpCode ("movz.s",       "D,S,t"),
    /* move is at the top of the table.  */
    M_OpCode ("msub.d",       "D,R,S,T"),
    M_OpCode ("msub.s",       "D,R,S,T"),
    M_OpCode ("msub",         "s,t"),
    M_OpCode ("msubu",        "s,t"),
    M_OpCode ("mtc0",         "t,G"),
    M_OpCode ("mtc1",         "t,S"),
    M_OpCode ("mtc1",         "t,G"),
    M_OpCode ("mtc2",         "t,G"),
    M_OpCode ("mtc3",         "t,G"),
    M_OpCode ("mthi",         "s"),
    M_OpCode ("mtlo",         "s"),
    M_OpCode ("mul.d",        "D,V,T"),
    M_OpCode ("mul.s",        "D,V,T"),
    M_OpCode ("mul",          "d,v,t"),
    M_OpCode ("mul",          "d,v,t"),
    M_OpCode ("mul",          "d,v,I"),
    M_OpCode ("mulo",         "d,v,t"),
    M_OpCode ("mulo",         "d,v,I"),
    M_OpCode ("mulou",        "d,v,t"),
    M_OpCode ("mulou",        "d,v,I"),
    M_OpCode ("mult",         "s,t"),
    M_OpCode ("multu",        "s,t"),
    M_OpCode ("neg",          "d,w"),
    M_OpCode ("negu",         "d,w"),
    M_OpCode ("neg.d",        "D,V"),
    M_OpCode ("neg.s",        "D,V"),
    M_OpCode ("nmadd.d",      "D,R,S,T"),
    M_OpCode ("nmadd.s",      "D,R,S,T"),
    M_OpCode ("nmsub.d",      "D,R,S,T"),
    M_OpCode ("nmsub.s",      "D,R,S,T"),
    /* nop is at the start of the table.  */
    M_OpCode ("nor",          "d,v,t"),
    M_OpCode ("nor",          "t,r,I"),
    M_OpCode ("not",          "d,v"),
    M_OpCode ("or",           "d,v,t"),
    M_OpCode ("or",           "t,r,I"),
    M_OpCode ("ori",          "t,r,i"),
    M_OpCode ("pref",         "k,o(b)"),
    M_OpCode ("prefx",        "h,t(b)"),
    M_OpCode ("recip.d",      "D,S"),
    M_OpCode ("recip.s",      "D,S"),
    M_OpCode ("rem",          "z,s,t"),
    M_OpCode ("rem",          "d,v,t"),
    M_OpCode ("rem",          "d,v,I"),
    M_OpCode ("remu",         "z,s,t"),
    M_OpCode ("remu",         "d,v,t"),
    M_OpCode ("remu",         "d,v,I"),
    M_OpCode ("rfe",          ""),
    M_OpCode ("rol",          "d,v,t"),
    M_OpCode ("rol",          "d,v,I"),
    M_OpCode ("ror",          "d,v,t"),
    M_OpCode ("ror",          "d,v,I"),
    M_OpCode ("round.l.d",    "D,S"),
    M_OpCode ("round.l.s",    "D,S"),
    M_OpCode ("round.w.d",    "D,S"),
    M_OpCode ("round.w.s",    "D,S"),
    M_OpCode ("rsqrt.d",      "D,S"),
    M_OpCode ("rsqrt.s",      "D,S"),
    M_OpCode ("sb",           "t,o(b)"),
    M_OpCode ("sb",           "t,A(b)"),
    M_OpCode ("sc",           "t,o(b)"),
    M_OpCode ("sc",           "t,A(b)"),
    M_OpCode ("scd",          "t,o(b)"),
    M_OpCode ("scd",          "t,A(b)"),
    M_OpCode ("sd",           "t,o(b)"),
    M_OpCode ("sd",           "t,o(b)"),
    M_OpCode ("sd",           "t,A(b)"),
    M_OpCode ("sdc1",         "T,o(b)"),
    M_OpCode ("sdc1",         "E,o(b)"),
    M_OpCode ("sdc1",         "T,A(b)"),
    M_OpCode ("sdc1",         "E,A(b)"),
    M_OpCode ("sdc2",         "E,o(b)"),
    M_OpCode ("sdc2",         "E,A(b)"),
    M_OpCode ("sdc3",         "E,o(b)"),
    M_OpCode ("sdc3",         "E,A(b)"),
    M_OpCode ("s.d",          "T,o(b)"),
    M_OpCode ("s.d",          "T,o(b)"),
    M_OpCode ("s.d",          "T,A(b)"),
    M_OpCode ("sdl",          "t,o(b)"),
    M_OpCode ("sdl",          "t,A(b)"),
    M_OpCode ("sdr",          "t,o(b)"),
    M_OpCode ("sdr",          "t,A(b)"),
    M_OpCode ("sdxc1",        "S,t(b)"),
    M_OpCode ("selsl",        "d,v,t"),
    M_OpCode ("selsr",        "d,v,t"),
    M_OpCode ("seq",          "d,v,t"),
    M_OpCode ("seq",          "d,v,I"),
    M_OpCode ("sge",          "d,v,t"),
    M_OpCode ("sge",          "d,v,I"),
    M_OpCode ("sgeu",         "d,v,t"),
    M_OpCode ("sgeu",         "d,v,I"),
    M_OpCode ("sgt",          "d,v,t"),
    M_OpCode ("sgt",          "d,v,I"),
    M_OpCode ("sgtu",         "d,v,t"),
    M_OpCode ("sgtu",         "d,v,I"),
    M_OpCode ("sh",           "t,o(b)"),
    M_OpCode ("sh",           "t,A(b)"),
    M_OpCode ("sle",          "d,v,t"),
    M_OpCode ("sle",          "d,v,I"),
    M_OpCode ("sleu",         "d,v,t"),
    M_OpCode ("sleu",         "d,v,I"),
    M_OpCode ("sllv",         "d,t,s"),
    M_OpCode ("sll",          "d,w,s"),
    M_OpCode ("sll",          "d,w,<"),
    M_OpCode ("slt",          "d,v,t"),
    M_OpCode ("slt",          "d,v,I"),
    M_OpCode ("slti",         "t,r,j"),
    M_OpCode ("sltiu",        "t,r,j"),
    M_OpCode ("sltu",         "d,v,t"),
    M_OpCode ("sltu",         "d,v,I"),
    M_OpCode ("sne",          "d,v,t"),
    M_OpCode ("sne",          "d,v,I"),
    M_OpCode ("sqrt.d",       "D,S"),
    M_OpCode ("sqrt.s",       "D,S"),
    M_OpCode ("srav",         "d,t,s"),
    M_OpCode ("sra",          "d,w,s"),
    M_OpCode ("sra",          "d,w,<"),
    M_OpCode ("srlv",         "d,t,s"),
    M_OpCode ("srl",          "d,w,s"),
    M_OpCode ("srl",          "d,w,<"),
    M_OpCode ("standby",      ""),
    M_OpCode ("sub",          "d,v,t"),
    M_OpCode ("sub",          "d,v,I"),
    M_OpCode ("sub.d",        "D,V,T"),
    M_OpCode ("sub.s",        "D,V,T"),
    M_OpCode ("subu",         "d,v,t"),
    M_OpCode ("subu",         "d,v,I"),
    M_OpCode ("suspend",      ""),
    M_OpCode ("sw",           "t,o(b)"),
    M_OpCode ("sw",           "t,A(b)"),
    M_OpCode ("swc0",         "E,o(b)"),
    M_OpCode ("swc0",         "E,A(b)"),
    M_OpCode ("swc1",         "T,o(b)"),
    M_OpCode ("swc1",         "E,o(b)"),
    M_OpCode ("swc1",         "T,A(b)"),
    M_OpCode ("swc1",         "E,A(b)"),
    M_OpCode ("s.s",          "T,o(b)"),
    M_OpCode ("s.s",          "T,A(b)"),
    M_OpCode ("swc2",         "E,o(b)"),
    M_OpCode ("swc2",         "E,A(b)"),
    M_OpCode ("swc3",         "E,o(b)"),
    M_OpCode ("swc3",         "E,A(b)"),
    M_OpCode ("swl",          "t,o(b)"),
    M_OpCode ("swl",          "t,A(b)"),
    M_OpCode ("scache",       "t,o(b)"),
    M_OpCode ("scache",       "t,A(b)"),
    M_OpCode ("swr",          "t,o(b)"),
    M_OpCode ("swr",          "t,A(b)"),
    M_OpCode ("invalidate",   "t,o(b)"),
    M_OpCode ("invalidate",   "t,A(b)"),
    M_OpCode ("swxc1",        "S,t(b)"),
    M_OpCode ("sync",         ""),
    M_OpCode ("syscall",      ""),
    M_OpCode ("syscall",      "B"),
    M_OpCode ("teqi",         "s,j"),
    M_OpCode ("teq",          "s,t"),
    M_OpCode ("teq",          "s,j"),
    M_OpCode ("teq",          "s,I"),
    M_OpCode ("tgei",         "s,j"),
    M_OpCode ("tge",          "s,t"),
    M_OpCode ("tge",          "s,j"),
    M_OpCode ("tge",          "s,I"),
    M_OpCode ("tgeiu",        "s,j"),
    M_OpCode ("tgeu",         "s,t"),
    M_OpCode ("tgeu",         "s,j"),
    M_OpCode ("tgeu",         "s,I"),
    M_OpCode ("tlbp",         ""),
    M_OpCode ("tlbr",         ""),
    M_OpCode ("tlbwi",        ""),
    M_OpCode ("tlbwr",        ""),
    M_OpCode ("tlti",         "s,j"),
    M_OpCode ("tlt",          "s,t"),
    M_OpCode ("tlt",          "s,j"),
    M_OpCode ("tlt",          "s,I"),
    M_OpCode ("tltiu",        "s,j"),
    M_OpCode ("tltu",         "s,t"),
    M_OpCode ("tltu",         "s,j"),
    M_OpCode ("tltu",         "s,I"),
    M_OpCode ("tnei",         "s,j"),
    M_OpCode ("tne",          "s,t"),
    M_OpCode ("tne",          "s,j"),
    M_OpCode ("tne",          "s,I"),
    M_OpCode ("trunc.l.d",    "D,S"),
    M_OpCode ("trunc.l.s",    "D,S"),
    M_OpCode ("trunc.w.d",    "D,S"),
    M_OpCode ("trunc.w.d",    "D,S,x"),
    M_OpCode ("trunc.w.d",    "D,S,t"),
    M_OpCode ("trunc.w.s",    "D,S"),
    M_OpCode ("trunc.w.s",    "D,S,x"),
    M_OpCode ("trunc.w.s",    "D,S,t"),
    M_OpCode ("uld",          "t,o(b)"),
    M_OpCode ("uld",          "t,A(b)"),
    M_OpCode ("ulh",          "t,o(b)"),
    M_OpCode ("ulh",          "t,A(b)"),
    M_OpCode ("ulhu",         "t,o(b)"),
    M_OpCode ("ulhu",         "t,A(b)"),
    M_OpCode ("ulw",          "t,o(b)"),
    M_OpCode ("ulw",          "t,A(b)"),
    M_OpCode ("usd",          "t,o(b)"),
    M_OpCode ("usd",          "t,A(b)"),
    M_OpCode ("ush",          "t,o(b)"),
    M_OpCode ("ush",          "t,A(b)"),
    M_OpCode ("usw",          "t,o(b)"),
    M_OpCode ("usw",          "t,A(b)"),
    M_OpCode ("xor",          "d,v,t"),
    M_OpCode ("xor",          "t,r,I"),
    M_OpCode ("xori",         "t,r,i"),
    M_OpCode ("waiti",        ""),
    M_OpCode ("wb",           "o(b)"),
    /*
     * No hazard protection on coprocessor instructions--they shouldn't
     * change the state of the processor and if they do it's up to the
     * user to put in nops as necessary.  These are at the end so that the
     * disasembler recognizes more specific versions first.
     * */
    M_OpCode ("c0",           "C"),
    M_OpCode ("c1",           "C"),
    M_OpCode ("c2",           "C"),
    M_OpCode ("c3",           "C"),
};

const size_t mips_opcodes_num = ((sizeof mips_opcodes) / (sizeof (mips_opcodes[0])));

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
    F_RegistersAdd (regs_result, v0, "$v0");
    F_RegistersAdd (regs_result, v1, "$v1");

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
