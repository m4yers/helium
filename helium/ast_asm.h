#ifndef AST_ASM_H_FEXZ1H5P
#define AST_ASM_H_FEXZ1H5P

#include <stdio.h>

#include "ext/util.h"
#include "ext/list.h"
#include "ext/bool.h"

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
*  Operands  *
**************/

typedef enum
{
    A_asmOpIntKind,
    A_asmOpRegNumKind,  // num register will be normalized by semantic analysis
    A_asmOpRegNameKind,
    A_asmOpMemKind,
} A_asmOpKind;

typedef struct A_asmOp_t
{
    struct A_loc_t loc;
    A_asmOpKind kind;
    union
    {
        // TODO make it size_t
        int integer;
        int num;
        const char * name;
    } u;
} * A_asmOp;

LIST_DEFINE (A_asmOpList, A_asmOp)
LIST_CONST_DEFINE (A_AsmOpList, A_asmOpList, A_asmOp)

A_asmOp A_AsmOpInt (A_loc loc, int imm);
A_asmOp A_AsmOpRegNum (A_loc loc, int num);
A_asmOp A_AsmOpRegName (A_loc loc, const char * name);

/****************
*  Statements  *
****************/

typedef enum
{
    A_asmStmInstKind
} A_asmStmKind;

typedef struct A_asmStmInst_t
{
    const char * opcode;
    A_asmOpList opList;
} * A_asmStmInst;

typedef struct A_asmStm_t
{
    struct A_loc_t loc;
    A_asmStmKind kind;
    union
    {
        struct A_asmStmInst_t inst;
    } u;
} * A_asmStm;

LIST_DEFINE (A_asmStmList, A_asmStm)
LIST_CONST_DEFINE (A_AsmStmList, A_asmStmList, A_asmStm)

A_asmStm A_AsmStmInst(A_loc loc, const char * opcode, A_asmOpList opList);

/*************
*  Printer  *
*************/

void AST_AsmPrint (FILE * out, A_asmStmList list, int d);

#endif /* end of include guard: AST_ASM_H_FEXZ1H5P */
