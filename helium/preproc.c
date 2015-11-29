#include <assert.h>

#include "ext/str.h"

#include "ast.h"
#include "preproc.h"

typedef struct PreProc_Context_t
{
    Program_Module module;
    S_table tenv;

} * PreProc_Context;

static A_exp TransExp (PreProc_Context context, A_exp exp);
static A_dec TransDec (PreProc_Context context, A_dec dec);

static A_scope TransScope (PreProc_Context context, A_scope scope)
{
    LIST_FOREACH (stm, scope->list)
    {
        switch (stm->kind)
        {
        case A_stmExp:
        {
            stm->u.exp = TransExp (context, stm->u.exp);
            break;
        }
        case A_stmDec:
        {
            stm->u.dec = TransDec (context, stm->u.dec);
            break;
        }
        }
    }

    return scope;
}

static A_exp TransExp (PreProc_Context context, A_exp exp)
{
    switch (exp->kind)
    {
    case A_seqExp:
    {
        if (!exp->u.seq)
        {
            return exp;
        }

        A_expList l = NULL;
        LIST_FOREACH (e, exp->u.seq)
        {
            LIST_PUSH (l, TransExp (context, l->head));
        }

        exp->u.seq = l;
        return exp;
    }
    // TODO real asm parser is badly needed
    // TODO drop pseudo
    case A_macroCallExp:
    {
        struct String_t name = String (exp->u.macro.name->name);

        if (String_Equal (&name, "panic"))
        {
            const char * str = exp->u.macro.args->head->u.stringg;

            A_expList l = NULL;

            // show message
            LIST_PUSH (l, A_AsmExp (&exp->loc, "li $v0, 4", NULL,
                                    U_StringList ("$v0", NULL), NULL));

            LIST_PUSH (l, A_AsmExp (&exp->loc, "li $a0, `s0", str,
                                    U_StringList ("$a0", NULL), NULL));

            LIST_PUSH (l, A_AsmExp (&exp->loc, "syscall", NULL, NULL,
                                    U_StringList ("$v0",
                                                  U_StringList ("$a0", NULL))));

            // exit program with status code 1
            LIST_PUSH (l, A_AsmExp (&exp->loc, "li $v0, 17", NULL,
                                    U_StringList ("$v0", NULL), NULL));

            LIST_PUSH (l, A_AsmExp (&exp->loc, "li $a0, 1", NULL,
                                    U_StringList ("$a0", NULL), NULL));

            LIST_PUSH (l, A_AsmExp (&exp->loc, "syscall", NULL, NULL,
                                    U_StringList ("$v0",
                                                  U_StringList ("$a0", NULL))));

            exp->kind  = A_seqExp;
            exp->u.seq = l;
        }
    }
    default:
    {
        return exp;
    }
    }
}

static A_dec TransDec (PreProc_Context context, A_dec dec)
{
    switch (dec->kind)
    {
    case A_functionDec:
    {
        dec->u.function.scope = TransScope (context, dec->u.function.scope);
        return dec;
    }
    default:
    {
        return dec;
    }
    }
}

int PreProc_Translate (Program_Module m)
{
    assert (m);

    struct PreProc_Context_t context;

    // Pre Processor will replace all macro nodes with the actul expressions
    LIST_FOREACH (dec, m->ast) TransDec (&context, dec);

    return Vector_Size (&m->errors.semant);
}
