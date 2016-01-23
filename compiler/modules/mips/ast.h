#ifndef AST_ASM_H_FEXZ1H5P
#define AST_ASM_H_FEXZ1H5P

#include <stdio.h>

#include "util/util.h"
#include "util/list.h"
#include "util/bool.h"
#include "util/str.h"

#include "core/symbol.h"
#include "core/ast.h"

#include "modules/helium/types.h"
#include "modules/helium/ast.h"

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
    A_asmOpRepKind,
    A_asmOpVarKind,
    A_asmOpLitKind,
    A_asmOpRegKind,
    A_asmOpMemKind,
} A_asmOpKind;

typedef enum
{
    A_asmOpUseSrc,
    A_asmOpUseDst
} A_asmOpUse;

typedef struct A_asmOp_t
{
    struct A_loc_t loc;
    A_asmOpKind kind;
    union
    {
        struct
        {
            A_asmOpUse use;
            int pos;
        } rep;
        A_var var;
        A_asmReg reg;
        struct
        {
            A_literal offset;
            struct A_asmOp_t * base;
        } mem;
        A_literal lit;
    } u;
} * A_asmOp;

LIST_DEFINE (A_asmOpList, A_asmOp)
LIST_CONST_DEFINE (A_AsmOpList, A_asmOpList, A_asmOp)

A_asmOp A_AsmOpRep (A_loc loc, const char * rep);
A_asmOp A_AsmOpVar (A_loc loc, A_var);
A_asmOp A_AsmOpLit (A_loc loc, A_literal lit);
A_asmOp A_AsmOpReg (A_loc loc, A_asmReg reg);
A_asmOp A_AsmOpMem (A_loc loc, A_literal offset, A_asmOp base);

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

typedef struct A_asmStmLab_t
{
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

    // FIXME not the correct place move to IR
    struct T_expList_t * dst;
    struct T_expList_t * src;
    struct T_stmList_t * pre;
    struct T_stmList_t * post;
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
