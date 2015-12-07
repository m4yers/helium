#include "ext/util.h"
#include "ext/bool.h"
#include "ext/list.h"
#include "ext/mem.h"

#include "symbol.h"
#include "escape.h"
#include "ast.h"

typedef struct EntryType
{
    int depth;
    bool * escape;

} * Entry;

static Entry Entry_New (int depth, bool * escape)
{
    Entry r = checked_malloc (sizeof (*r));
    r->depth = depth;
    r->escape = escape;
    *r->escape = FALSE;
    return r;
}

static void TraverseDec (S_table env, int depth, A_dec dec);
static void TraverseVar (S_table env, int depth, A_var var);
static void TraverseExp (S_table env, int depth, A_exp exp);

static void TraverseScope (S_table env, int depth, A_scope scope)
{
    LIST_FOREACH (stm, scope->list)
    {
        switch (stm->kind)
        {
        case A_stmExp:
        {
            TraverseExp (env, depth, stm->u.exp);
            break;
        }
        case A_stmDec:
        {
            TraverseDec (env, depth, stm->u.dec);
            break;
        }
        }
    }
}

static void TraverseDec (S_table env, int depth, A_dec dec)
{
    switch (dec->kind)
    {
    case A_varDec:
    {
        S_Enter (env, dec->u.var.var, Entry_New (depth, &dec->u.var.escape));
        if (dec->u.var.init)
        {
            TraverseExp (env, depth, dec->u.var.init);
        }
        break;
    }
    case A_functionDec:
    {
        S_BeginScope (env);

        for (A_fieldList f = dec->u.function.params; f; f = f->tail)
        {
            S_Enter (env, f->head->name, Entry_New (depth + 1, &f->head->escape));
        }

        TraverseScope (env, depth + 1, dec->u.function.scope);

        S_EndScope (env);

        break;
    }
    default:
        ;
    }

}

static void TraverseVar (S_table env, int depth, A_var var)
{
    switch (var->kind)
    {
    case A_simpleVar:
    {
        Entry e = (Entry) S_Look (env, var->u.simple);
        if (e && depth > e->depth)
        {
            *e->escape = TRUE;
        }
        break;
    }
    case A_fieldVar:
    {
        TraverseVar (env, depth, var->u.field.var);
        break;
    }
    case A_subscriptVar:
    {
        TraverseVar (env, depth, var->u.subscript.var);
        break;
    }
    }
}

static void TraverseExp (S_table env, int depth, A_exp exp)
{
    switch (exp->kind)
    {
    case A_seqExp:
    {
        for (A_expList l = exp->u.seq; l; l = l->tail)
        {
            TraverseExp (env, depth, l->head);
        }
        break;
    }
    case A_varExp:
    {
        TraverseVar (env, depth, exp->u.var);
        break;
    }
    case A_retExp:
    {
        TraverseExp (env, depth, exp->u.ret);
    }
    case A_callExp:
    {
        for (A_expList l = exp->u.call.args; l; l = l->tail)
        {
            TraverseExp (env, depth, l->head);
        }
        break;
    }
    case A_opExp:
    {
        TraverseExp (env, depth, exp->u.op.left);
        TraverseExp (env, depth, exp->u.op.right);
        break;
    }
    /* case A_arrayExp: */
    /* { */
    /*     TraverseExp (env, depth, exp->u.array.size); */
    /*     TraverseExp (env, depth, exp->u.array.init); */
    /*     break; */
    /* } */
    case A_recordExp:
    {
        for (A_efieldList l = exp->u.record.fields; l; l = l->tail)
        {
            TraverseExp (env, depth, l->head->exp);
        }
        break;
    }
    case A_assignExp:
    {
        TraverseVar (env, depth, exp->u.assign.var);
        TraverseExp (env, depth, exp->u.assign.exp);
        break;
    }
    case A_ifExp:
    {
        TraverseExp (env, depth, exp->u.iff.test);
        if (exp->u.iff.tr)
        {
            TraverseScope (env, depth, exp->u.iff.tr);
        }
        if (exp->u.iff.fl)
        {
            TraverseScope (env, depth, exp->u.iff.fl);
        }
        break;
    }
    case A_whileExp:
    {
        TraverseExp (env, depth, exp->u.whilee.test);
        TraverseScope (env, depth, exp->u.whilee.body);
        break;
    }
    case A_forExp:
    {
        TraverseExp (env, depth, exp->u.forr.lo);
        TraverseExp (env, depth, exp->u.forr.hi);
        S_BeginScope (env);
        S_Enter (env, exp->u.forr.var, Entry_New (depth + 1, &exp->u.forr.escape));
        TraverseScope (env, depth, exp->u.forr.body);
        S_EndScope (env);
        break;
    }
    default:
        ;
    }
}

void Escape_Find (A_decList list)
{
    S_table env = S_Empty();
    LIST_FOREACH (dec, list)
    {
        TraverseDec (env, 0, dec);
    }
}
