#include <assert.h>

#include "ext/util.h"
#include "ext/mem.h"
#include "ext/list.h"
#include "ext/bool.h"

#include "symbol.h"

#include "ast_asm.h"

/**************
*  Operands  *
**************/

A_asmOp A_AsmOpImm (A_loc loc, int imm)
{
    A_asmOp p = checked_malloc (sizeof (*p));
    p->kind = A_asmOpRegNum;
    p->loc = *loc;
    p->u.imm = imm;
    return p;
}

A_asmOp A_AsmOpRegNum (A_loc loc, int num)
{
    A_asmOp p = checked_malloc (sizeof (*p));
    p->kind = A_asmOpRegNum;
    p->loc = *loc;
    p->u.num = num;
    return p;
}

A_asmOp A_AsmOpRegName (A_loc loc, const char * name)
{
    A_asmOp p = checked_malloc (sizeof (*p));
    p->kind = A_asmOpRegName;
    p->loc = *loc;
    p->u.name = name;
    return p;
}

/******************
*  Instructions  *
******************/

A_asmStm A_AsmStmInst (A_loc loc, const char * code, A_asmOpList opList)
{
    A_asmStm p = checked_malloc (sizeof (*p));
    p->loc = *loc;
    p->kind = A_asmStmInstKind;
    p->u.inst.code = code;
    p->u.inst.opList = opList;
    return p;
}

/****************
*  Statements  *
****************/
