#ifndef AST_ASM_H_FEXZ1H5P
#define AST_ASM_H_FEXZ1H5P

#include <stdio.h>

#include "ext/util.h"
#include "ext/list.h"
#include "ext/bool.h"
#include "ext/str.h"

#include "ast.h"
#include "ast_asm.h"
#include "symbol.h"

/********************
*  ASM node kinds  *
********************/

typedef enum
{
    A_asmStmNode,
    A_asmOpNode, A_asmRegNode,
    A_asmLabelNode
} A_asmNodeKind;

/**************
*  Register  *
**************/

typedef enum
{
    A_asmRegNumKind,
    A_asmRegNameKind
} A_asmRegKind;

typedef struct A_asmReg_t
{
    struct A_loc_t loc;
    A_asmRegKind kind;
    union
    {
        int num;
        const char * name;
    } u;
} * A_asmReg;

A_asmReg A_AsmRegNum (A_loc loc, int num);
A_asmReg A_AsmRegName (A_loc loc, const char * name);

/**************
*  Operands  *
**************/

typedef enum
{
    A_asmOpSymKind,
    A_asmOpIntKind,
    A_asmOpRegKind,
    A_asmOpMemKind,
} A_asmOpKind;

typedef struct A_asmOp_t
{
    struct A_loc_t loc;
    A_asmOpKind kind;
    union
    {
        S_symbol sym;
        signed long integer;
        A_asmReg reg;
        struct
        {
            signed long offset;
            A_asmReg base;
        } mem;
    } u;
} * A_asmOp;

LIST_DEFINE (A_asmOpList, A_asmOp)
LIST_CONST_DEFINE (A_AsmOpList, A_asmOpList, A_asmOp)

A_asmOp A_AsmOpSym  (A_loc loc, S_symbol sym);
A_asmOp A_AsmOpInt (A_loc loc, signed long imm);
A_asmOp A_AsmOpReg (A_loc loc, A_asmReg reg);
A_asmOp A_AsmOpMem (A_loc loc, signed long offset, A_asmReg base);

/****************
*  Statements  *
****************/

typedef enum
{
    A_asmStmInstKind,
    A_asmStmLabKind
} A_asmStmKind;

typedef struct A_asmStmInst_t
{
    const char * opcode;
    A_asmOpList opList;
} * A_asmStmInst;

typedef struct A_asmStmLab_t {
    const char * name;
} * A_asmStmLab;

typedef struct A_asmStm_t
{
    struct A_loc_t loc;
    A_asmStmKind kind;
    union
    {
        struct A_asmStmLab_t lab;
        struct A_asmStmInst_t inst;
    } u;
} * A_asmStm;

LIST_DEFINE (A_asmStmList, A_asmStm)
LIST_CONST_DEFINE (A_AsmStmList, A_asmStmList, A_asmStm)

A_asmStm A_AsmStmInst (A_loc loc, const char * opcode, A_asmOpList opList);
A_asmStm A_AsmStmLab (A_loc loc, const char * name);

/*************
*  Emitter  *
*************/

void AST_AsmEmitLine (String out, A_asmStm stm);

/*************
*  Printer  *
*************/

void AST_AsmPrint (FILE * out, A_asmStmList list, int d);

#endif /* end of include guard: AST_ASM_H_FEXZ1H5P */
