#include <assert.h>
#include <inttypes.h>

#include "util/util.h"
#include "util/mem.h"
#include "util/list.h"
#include "util/bool.h"

#include "modules/mips/ast.h"

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

A_asmOp A_AsmOpLit (A_loc loc, A_literal lit)
{
    A_asmOp p = checked_malloc (sizeof (*p));
    p->kind = A_asmOpLitKind;
    p->loc = *loc;
    p->u.lit = lit;
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

A_asmOp A_AsmOpTmp (A_loc loc, S_symbol sym)
{
    A_asmOp p = checked_malloc (sizeof (*p));
    p->kind = A_asmOpTmpKind;
    p->loc = *loc;
    p->u.tmp = sym;
    return p;
}

A_asmOp A_AsmOpLab (A_loc loc, S_symbol sym)
{
    A_asmOp p = checked_malloc (sizeof (*p));
    p->kind = A_asmOpLabKind;
    p->loc = *loc;
    p->u.lab = sym;
    return p;
}

A_asmOp A_AsmOpMem (A_loc loc, A_literal offset, A_asmOp base)
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

A_asmStm A_AsmStmLab (A_loc loc, S_symbol sym, bool meta)
{
    A_asmStm p = checked_malloc (sizeof (*p));
    p->loc = *loc;
    p->kind = A_asmStmLabKind;
    p->u.lab.sym = sym;
    p->u.lab.meta = meta;
    return p;
}

/**********************************************************************
*                           Usage Contexts                           *
**********************************************************************/

const struct AST_asmContextRecord_t mips_ast_opd_contexts[] =
{
    AST_AsmContextRec ("Replacement", A_asmOpRepKind, A_asmContextFuncKind),
    AST_AsmContextRec ("Variable", A_asmOpVarKind, A_asmContextFuncKind),
    AST_AsmContextRec ("Literal", A_asmOpLitKind, A_asmContextFuncKind | A_asmContextGlobKind),
    AST_AsmContextRec ("Register", A_asmOpRegKind, A_asmContextFuncKind | A_asmContextGlobKind),
    AST_AsmContextRec ("Meta Register", A_asmOpTmpKind, A_asmContextFuncKind),
    AST_AsmContextRec ("Label", A_asmOpLabKind, A_asmContextFuncKind | A_asmContextGlobKind),
    AST_AsmContextRec ("Memory Access", A_asmOpMemKind, A_asmContextFuncKind | A_asmContextGlobKind),
};

#define TOTAL_ELEMENTS(array) (sizeof(array) / sizeof(array[0]))

static const struct AST_asmContextRecord_t * GetMipsAstOpdCntxDesc (A_asmOpKind kind)
{
    for (size_t i = 0; i < TOTAL_ELEMENTS (mips_ast_opd_contexts); ++i)
    {
        if (mips_ast_opd_contexts[i].kind == kind)
        {
            return &mips_ast_opd_contexts[i];
        }
    }

    assert (0);
    return NULL;
}

/**********************************************************************
*                              Analysis                              *
**********************************************************************/

static Error ValidateOpdContext (A_asmOp opd, A_asmContext cntx)
{
    const struct AST_asmContextRecord_t * desc = GetMipsAstOpdCntxDesc (opd->kind);
    if ((desc->mask & cntx) == 0)
    {
        // TODO organize error codes somehow
        return Error_NewPtr (&opd->loc, 3200,
                             "You cannot use %s operand in this context",
                             desc->name);
    }

    if (opd->kind == A_asmOpMemKind)
    {
        return ValidateOpdContext(opd->u.mem.base, cntx);
    }

    return NULL;
}

ErrorList A_AsmValidateContext (A_asmStmList stml, A_asmContext cntx)
{
    ErrorList rslt = NULL;

    LIST_FOREACH (stm, stml)
    {
        switch (stm->kind)
        {
        case A_asmStmInstKind:
        {
            LIST_FOREACH (opd, stm->u.inst.opList)
            {
                Error err = ValidateOpdContext (opd, cntx);
                if (err)
                {
                    LIST_PUSH (rslt, err);
                    continue;
                }
            }
            break;
        }
        case A_asmStmLabKind:
        {
            rslt = FALSE;
            break;
        }
        }
    }

    return rslt;
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
    case A_asmOpLitKind:
    {
        fprintf (out, "Lit(");
        AST_PrintLiteral (out, op->u.lit, 0);
        fprintf (out, ")");
        break;
    }
    case A_asmOpRegKind:
    {
        PrintReg (out, op->u.reg);
        break;
    }
    case A_asmOpTmpKind:
    {
        fprintf (out, "Tmp(%s)", op->u.tmp->name);
        break;
    }
    case A_asmOpLabKind:
    {
        fprintf (out, "Lab(%s)", op->u.lab->name);
        break;
    }
    case A_asmOpMemKind:
    {
        fprintf (out, "Mem(%"PRIdMAX",", op->u.mem.offset->u.ival);
        PrintOp (out, op->u.mem.base, d);
        fprintf (out, ")");
        break;
    }
    case A_asmOpRepKind:
    {
        assert (0);
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
    fprintf (out, "Label(%s)", lab->sym->name);
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
