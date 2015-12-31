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

A_asmOp A_AsmOpInt (A_loc loc, int integer)
{
    A_asmOp p = checked_malloc (sizeof (*p));
    p->kind = A_asmOpIntKind;
    p->loc = *loc;
    p->u.integer = integer;
    return p;
}

A_asmOp A_AsmOpRegNum (A_loc loc, int num)
{
    A_asmOp p = checked_malloc (sizeof (*p));
    p->kind = A_asmOpRegNumKind;
    p->loc = *loc;
    p->u.num = num;
    return p;
}

A_asmOp A_AsmOpRegName (A_loc loc, const char * name)
{
    A_asmOp p = checked_malloc (sizeof (*p));
    p->kind = A_asmOpRegNameKind;
    p->loc = *loc;
    p->u.name = name;
    return p;
}

/******************
*  Instructions  *
******************/

A_asmStm A_AsmStmInst (A_loc loc, const char * opcode, A_asmOpList opList)
{
    A_asmStm p = checked_malloc (sizeof (*p));
    p->loc = *loc;
    p->kind = A_asmStmInstKind;
    p->u.inst.opcode = opcode;
    p->u.inst.opList = opList;
    return p;
}

/**********************************************************************
*                              Printer                               *
**********************************************************************/

static void PrintInst (FILE * out, A_asmStmInst inst, int d);
static void PrintOp (FILE * out, A_asmOp op, int d);

static void PrintIndent (FILE * out, int d)
{
    int i;

    for (i = 0; i < d; i++)
    {
        fprintf (out, "  ");
    }
}

void AST_AsmPrint (FILE * out, A_asmStmList list, int d)
{
    PrintIndent (out, d);

    fprintf (out, "Asm(");

    size_t size = LIST_SIZE(list);
    LIST_FOREACH (stm, list)
    {
        fprintf (out, "\n");
        switch (stm->kind)
        {
        case A_asmStmInstKind:
        {
            PrintInst (out, &stm->u.inst, d + 1);
            break;
        }
        }

        if (--size)
        {
            fprintf (out, ",\n");
        }
    }

    fprintf (out, ")");
}

void PrintInst (FILE * out, A_asmStmInst inst, int d)
{
    PrintIndent (out, d);

    fprintf (out, "Inst(%s", inst->opcode);

    LIST_FOREACH (op, inst->opList)
    {
        fprintf (out, ",");
        PrintOp (out, op, d);
    }

    fprintf (out, ")");
}

void PrintOp (FILE * out, A_asmOp op, int d)
{
    (void) d;

    switch (op->kind)
    {
    case A_asmOpIntKind:
    {
        fprintf (out, "Imm(%d)", op->u.integer);
        break;
    }
    case A_asmOpRegNumKind:
    {
        fprintf (out, "RegNum(%d)", op->u.num);
        break;
    }
    case A_asmOpRegNameKind:
    {
        fprintf (out, "RegName(%s)", op->u.name);
        break;
    }
    }
}
