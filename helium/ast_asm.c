#include <assert.h>

#include "ext/util.h"
#include "ext/mem.h"
#include "ext/list.h"
#include "ext/bool.h"

#include "symbol.h"

#include "ast_asm.h"
#include "ast_helium.h"


/**************
*  Register  *
**************/

A_asmReg A_AsmRegNum (A_loc loc, int num)
{
    A_asmReg p = checked_malloc (sizeof (*p));
    p->kind = A_asmRegNumKind;
    p->loc = *loc;
    p->u.num = num;
    return p;
}

A_asmReg A_AsmRegName (A_loc loc, const char * name)
{
    A_asmReg p = checked_malloc (sizeof (*p));
    p->kind = A_asmRegNameKind;
    p->loc = *loc;
    p->u.name = name;
    return p;
}

/**************
*  Operands  *
**************/

A_asmOp A_AsmOpVar (A_loc loc, A_var var)
{
    A_asmOp p = checked_malloc (sizeof (*p));
    p->kind = A_asmOpVarKind;
    p->loc = *loc;
    p->u.var = var;
    return p;
}

A_asmOp A_AsmOpInt (A_loc loc, signed long integer)
{
    A_asmOp p = checked_malloc (sizeof (*p));
    p->kind = A_asmOpIntKind;
    p->loc = *loc;
    p->u.integer = integer;
    return p;
}

A_asmOp A_AsmOpReg (A_loc loc, A_asmReg reg)
{
    A_asmOp p = checked_malloc (sizeof (*p));
    p->kind = A_asmOpRegKind;
    p->loc = *loc;
    p->u.reg = reg;
    return p;
}

A_asmOp A_AsmOpMem (A_loc loc, signed long offset, A_asmReg base)
{
    A_asmOp p = checked_malloc (sizeof (*p));
    p->kind = A_asmOpMemKind;
    p->loc = *loc;
    p->u.mem.offset = offset;
    p->u.mem.base = base;
    return p;
}

/******************
*    Statements  *
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

A_asmStm A_AsmStmLab (A_loc loc, const char * name)
{
    A_asmStm p = checked_malloc (sizeof (*p));
    p->loc = *loc;
    p->kind = A_asmStmLabKind;
    p->u.lab.name = name;
    return p;
}

/**********************************************************************
*                              Emitter                               *
**********************************************************************/

static char emit_buf[512];

static void EmitInst (String out, A_asmStmInst inst)
{
    sprintf (emit_buf, "%-6s", inst->opcode);
    String_Append (out, emit_buf);
    size_t opsSize = LIST_SIZE (inst->opList);
    LIST_FOREACH (op, inst->opList)
    {
        switch (op->kind)
        {
        case A_asmOpRepKind:
        {
            if (op->u.rep.use == A_asmOpUseSrc)
            {
                sprintf (emit_buf, "`s%d", op->u.rep.pos);
            }
            else
            {
                sprintf (emit_buf, "`d%d", op->u.rep.pos);
            }
            break;
        }
        case A_asmOpIntKind:
        {
            sprintf (emit_buf, "%ld", op->u.integer);
            break;
        }
        case A_asmOpRegKind:
        {
            //NOTE reg has to be normalized by this point
            sprintf (emit_buf, "$%s", op->u.reg->u.name);
            break;
        }
        case A_asmOpMemKind:
        {
            sprintf (emit_buf, "%ld($%s),",
                     op->u.mem.offset,
                     op->u.mem.base->u.name);
            break;
        }
        default:
        {
            assert (0);
        }
        }

        String_Append (out, emit_buf);

        if (--opsSize)
        {
            String_Append (out, ", ");
        }
    }
}

static void EmitLabel (String out, A_asmStmLab lab)
{
    sprintf (emit_buf, "%s:", lab->name);
    String_Append (out, emit_buf);
}

void AST_AsmEmitLine (String out, A_asmStm stm)
{
    switch (stm->kind)
    {
    case A_asmStmInstKind:
    {
        EmitInst (out, &stm->u.inst);
        break;
    }
    case A_asmStmLabKind:
    {
        EmitLabel (out, &stm->u.lab);
        break;
    }
    }
}

/**********************************************************************
*                              Printer                               *
**********************************************************************/

static void PrintIndent (FILE * out, int d)
{
    int i;

    for (i = 0; i < d; i++)
    {
        fprintf (out, "  ");
    }
}

static void PrintReg (FILE * out, A_asmReg reg)
{
    switch (reg->kind)
    {
    case A_asmRegNumKind:
    {
        fprintf (out, "RegNum(%d)", reg->u.num);
        break;
    }
    case A_asmRegNameKind:
    {
        fprintf (out, "RegName(%s)", reg->u.name);
        break;
    }
    }
}

static void PrintOp (FILE * out, A_asmOp op, int d)
{
    (void) d;

    switch (op->kind)
    {
    case A_asmOpVarKind:
    {
        fprintf (out, "Var(");
        AST_PrintVar (out, (A_var)op->u.var, 0);
        fprintf (out, ")");
        break;
    }
    case A_asmOpIntKind:
    {
        fprintf (out, "Int(%ld)", op->u.integer);
        break;
    }
    case A_asmOpRegKind:
    {
        PrintReg (out, op->u.reg);
        break;
    }
    case A_asmOpMemKind:
    {
        fprintf (out, "Mem(%ld,", op->u.mem.offset);
        PrintReg (out, op->u.mem.base);
        fprintf (out, ")");
    }
    }
}

static void PrintInst (FILE * out, A_asmStmInst inst, int d)
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

static void PrintLabel (FILE * out, A_asmStmLab lab, int d)
{
    PrintIndent (out, d);
    fprintf (out, "Label(%s)", lab->name);
}

void AST_AsmPrint (FILE * out, A_asmStmList list, int d)
{
    PrintIndent (out, d);

    fprintf (out, "Asm(\n");

    size_t size = LIST_SIZE (list);
    LIST_FOREACH (stm, list)
    {
        switch (stm->kind)
        {
        case A_asmStmInstKind:
        {
            PrintInst (out, &stm->u.inst, d + 1);
            break;
        }
        case A_asmStmLabKind:
        {
            PrintLabel (out, &stm->u.lab, d + 1);
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
