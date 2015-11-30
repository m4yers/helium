#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include "ext/table.h"
#include "ext/list.h"

#include "semant.h"
#include "ast.h"
#include "types.h"
#include "symbol.h"
#include "escape.h"
#include "error.h"
#include "env.h"

#define ERROR_PUSH(line, loc, code, format, ...)                         \
    {                                                                    \
        Vector_PushBack(&context->module->errors.semant,                  \
                Error_New(&loc, code, format, __VA_ARGS__));              \
    }                                                                    \

#define ERROR_WRONG_TYPE(expected, actual, loc)                          \
    ERROR_PUSH(                                                          \
        __LINE__,                                                        \
        loc,                                                             \
        3001,                                                            \
        "Expected '%s', got '%s'",                                       \
        expected->meta.name,                                             \
        actual->meta.name);                                              \

#define ERROR_MALFORMED_EXP(loc, text)                                   \
    ERROR_PUSH(                                                          \
        __LINE__,                                                        \
        loc,                                                             \
        3002,                                                            \
        "Malformed exprassion: %s",                                      \
        text);                                                           \

#define ERROR_INVALID_TYPE(loc, name)                                    \
    ERROR_PUSH(                                                          \
        __LINE__,                                                        \
        loc,                                                             \
        3003,                                                            \
        "Invalid type '%s'",                                             \
        S_Name(name));                                                   \

#define ERROR_UNKNOWN_TYPE(loc, name)                                    \
    ERROR_PUSH(                                                          \
        __LINE__,                                                        \
        loc,                                                             \
        3004,                                                            \
        "Unknown type '%s'",                                             \
        S_Name(name));                                                   \

#define ERROR_UNKNOWN_VAR(var)                                           \
    ERROR_PUSH(                                                          \
        __LINE__,                                                        \
        var->loc,                                                        \
        3005,                                                            \
        "Unknown var '%s'",                                              \
        S_Name(var->u.simple));                                          \

typedef struct Semant_ExpType
{
    Tr_exp exp;
    Ty_ty ty;

} Semant_Exp;

static bool is_auto (Ty_ty ty)
{
    return ty == Ty_Auto();
}

static bool is_invalid (Ty_ty ty)
{
    return ty == Ty_Invalid();
}

static bool is_unknown (Ty_ty ty)
{
    return ty->kind == Ty_unknown;
}

static bool is_truetype (Ty_ty ty)
{
    return !is_invalid (ty) && !is_unknown (ty);
}

//static bool is_unknown (Ty_ty ty)
//{
//    return ty->kind == Ty_unknown;
//}

static bool is_int (Semant_Exp exp)
{
    return exp.ty == Ty_Int();
}

static bool is_string (Semant_Exp exp)
{
    return exp.ty == Ty_String();
}

static bool same_type (Semant_Exp a, Semant_Exp b)
{
    return a.ty == b.ty;
}

static Semant_Exp Expression_New (Tr_exp exp, Ty_ty ty)
{
    Semant_Exp r;
    r.exp = exp;
    r.ty = ty;
    return r;
}

static Ty_ty GetActualType (Ty_ty ty)
{
    if (ty->kind == Ty_name)
    {
        if (ty->u.name.ty == NULL)
        {
            printf ("NULL");
        }
        return ty->u.name.ty;
    }
    return ty;
}

static Semant_Exp TransExp (Semant_Context context, A_exp exp);
static Tr_exp TransDec (Semant_Context context, A_dec dec);

static Semant_Exp TransScope (Semant_Context context, A_scope scope)
{
    Semant_Exp r = { Tr_Void(), Ty_Void() };
    Tr_exp seq = NULL;
    LIST_FOREACH (stm, scope->list)
    {
        switch (stm->kind)
        {
        case A_stmExp:
        {
            r = TransExp (context, stm->u.exp);
            break;
        }
        case A_stmDec:
        {
            r.exp = TransDec (context, stm->u.dec);
            r.ty = Ty_Void();
            break;
        }
        }

        seq = Tr_Seq (seq, r.exp);
    }

    return Expression_New (seq, r.ty);
}

static Ty_ty TransTyp (Semant_Context context, A_ty ty)
{
    assert (ty);

    Ty_ty e_invalid = Ty_Invalid();
    Ty_ty type;

    switch (ty->kind)
    {
    case A_nameTy:
    {
        type = (Ty_ty)S_Look (context->tenv, ty->u.name);
        if (!type)
        {
            ERROR_UNKNOWN_TYPE (ty->loc, ty->u.name);
            return Ty_Unknown (S_Name (ty->u.name));

        }

        return type;
    }
    case A_arrayTy:
    {
        // SHIT well u did it again
        type = (Ty_ty)S_Look (context->tenv, ty->u.array->head->u.var->u.simple);
        int size = ty->u.array->tail->head->u.intt;
        return Ty_Array (type, size);
    }
    case A_recordTy:
    {
        Ty_fieldList tlist = NULL;
        for (A_fieldList l = ty->u.record; l; l = l->tail)
        {
            A_field field = l->head;
            if (field->type->kind == A_nameTy)
            {
                type = (Ty_ty)S_Look (context->tenv, field->type->u.name);
            }
            else
            {
                type = TransTyp (context, field->type);
            }

            // FIXME fix type tests
            /* if (!type) */
            /* { */
            /*     ERROR_UNKNOWN_TYPE (field->loc, field->typ); */
            /*     type = Ty_Unknown (S_Name (field->typ)); */
            /* } */

            LIST_PUSH (tlist, Ty_Field (field->name, type))
        }

        return Ty_Record (tlist);
    }
    }

    return e_invalid;
}

// TODO you need to do it in two passes: 1 - names, 2 - definitions
static Tr_exp TransDec (Semant_Context context, A_dec dec)
{
    switch (dec->kind)
    {
    case A_typeDec:
    {
        S_symbol name = dec->u.type.name;
        A_ty type = dec->u.type.type;
        S_Enter (context->tenv, name, Ty_Name (name, TransTyp (context, type)));
        return Tr_Void();
    }
    case A_varDec:
    {
        Ty_ty ty = NULL;
        Semant_Exp sexp = { NULL, NULL };

        if (dec->u.var.init)
        {
            sexp = TransExp (context, dec->u.var.init);
            if (is_invalid (sexp.ty))
            {
                // TODO proper error here
                printf ("VarDec: invalid expresison type\n");
                return Tr_Void();
            }
        }

        if (dec->u.var.type)
        {
            ty = TransTyp (context, dec->u.var.type);
            if (is_invalid (ty))
            {
                // TODO error proper here
                printf ("VarDec: invalid type\n");
                return Tr_Void();
            }
            ty = GetActualType (ty);
        }

        Tr_access access;
        if (sexp.exp)
        {
            // alloc handle only
            access = Tr_Alloc (context->level, Ty_Int(), dec->u.var.escape);
            sexp.exp = Tr_Assign (Tr_SimpleVar (access, context->level), sexp.exp);

            if (ty && ty != sexp.ty)
            {
                ERROR_WRONG_TYPE (ty, sexp.ty, dec->loc);
                return Tr_Void();
            }
            else
            {
                ty = sexp.ty;
            }
        }
        else
        {
            // alloc uninitialized array
            access = Tr_Alloc (context->level, GetActualType (ty), dec->u.var.escape);

        }

        // anonymous record
        // HMM do i need this?
        // YES for external unit linkage
        /* if (ty->kind == Ty_record) */
        /* { */
        /*     S_symbol name = S_Symbol ("__scope_anon_rec_0"); */
        /*     ty = Ty_Name (name, ty); */
        /*     S_Enter (context->tenv, name, ty); */
        /* } */

        S_Enter (context->venv, dec->u.var.var, Env_VarEntryNew (access, ty));

        return sexp.exp;
    }
    case A_functionDec:
    {
        Ty_ty rty = NULL;
        if (dec->u.function.type)
        {
            rty = TransTyp (context, dec->u.function.type);
            if (!rty)
            {
                // expects symbols
                /* ERROR_UNKNOWN_TYPE (dec->loc, dec->u.function.type); */
                /* rty = Ty_Unknown (dec->u.function.type); */
            }
        }
        else
        {
            rty = Ty_Auto();
        }

        Ty_tyList formals = NULL;
        U_boolList escapes = NULL;
        LIST_FOREACH (field, dec->u.function.params)
        {
            Ty_ty fty;
            if (field->type->kind == A_nameTy)
            {
                fty = (Ty_ty)S_Look (context->tenv, field->type->u.name);
            }
            else
            {
                fty = TransTyp (context, field->type);
            }

            // FIXME fix tests here
            /* if (!fty) */
            /* { */
            /*     ERROR_UNKNOWN_TYPE (dec->loc, field->type); */
            /*     fty = Ty_Unknown (S_Name (field->typ)); */
            /* } */

            LIST_PUSH (formals, fty);
            LIST_PUSH (escapes, field->escape);
        }

        Temp_label label = Tr_ScopedLabel (context->level, dec->u.function.name->name);
        Env_Entry entry = Env_FunEntryNew (
                              Tr_NewLevel (context->level, label, escapes),
                              label, formals, rty);

        S_Enter (context->venv, dec->u.function.name, entry);

        Tr_level current = context->level;

        S_BeginScope (context->venv);
        S_BeginScope (context->tenv);

        {
            A_fieldList f = dec->u.function.params;
            Tr_accessList al = Tr_Formals (entry->u.fun.level);
            Ty_tyList t = entry->u.fun.formals;
            for (; f; f = f->tail, t = t->tail, al = al->tail)
            {
                S_Enter (context->venv, f->head->name,
                         Env_VarEntryNew (
                             al->head,
                             t->head));
            }
        }

        context->level = entry->u.fun.level;
        Semant_Exp ety = TransScope (context, dec->u.function.scope);

        Tr_ProcEntryExit (context, context->level, ety.exp);

        if (is_auto (rty))
        {
            entry->u.fun.result = ety.ty;
        }
        // FIXME wtf is this?
        else if (!is_unknown (rty) && !is_invalid (rty) && rty != ety.ty)
        {
            ERROR_WRONG_TYPE (rty, ety.ty, dec->loc);
        }

        S_EndScope (context->venv);
        S_EndScope (context->tenv);

        context->level = current;

        return Tr_Void();
    }
    }
}

static Semant_Exp TransVar (Semant_Context context, A_var var)
{
    Semant_Exp e_invalid = {NULL, Ty_Invalid()};

    /*
     * indicates nesting level, if level > 0 we just calculate correct offset, once we got back
     * to the original call(level==0) we read from the offset(base), e.g:
     *
     * Expresison:
     * foo.bar.x;
     *
     * Looks like this inside AST:
     * (((foo).bar).x)
     *
     * And parsed as:
     * $1 = foo    -> T_Temp(foo) # or anything that results as register
     * $2 = $1.bar -> T_Binop(T_plus, $1 + Offset(bar))
     * $3 = $1.x   -> T_Mem(T_Binop(T_plus, $2 + Offset(x)))
     */
    static int level = 0;

    switch (var->kind)
    {
    case A_simpleVar:
    {
        Env_Entry e = (Env_Entry)S_Look (context->venv, var->u.simple);
        if (e && e->kind == Env_varEntry)
        {
            return Expression_New (
                       Tr_SimpleVar (e->u.var.access, context->level),
                       GetActualType (e->u.var.ty));
        }
        else
        {
            ERROR_UNKNOWN_VAR (var);
            return e_invalid;
        }
    }
    case A_fieldVar:
    {
        level++;
        Semant_Exp ety = TransVar (context, var->u.field.var);
        level--;
        Ty_ty ty = ety.ty;

        if (!is_truetype (ty) || ty->kind != Ty_record)
        {
            // TODO:  proper error here
            return e_invalid;
        }

        LIST_FOREACH (f, ty->u.record)
        {
            if (f->name == var->u.field.sym)
            {
                return Expression_New (
                           Tr_FieldVar (ety.exp, ty, f->name, level == 0),
                           GetActualType (GetActualType (f->ty)));
            }
        }

        return e_invalid;
    }
    case A_subscriptVar:
    {
        level++;
        Semant_Exp ety = TransVar (context, var->u.subscript.var);
        level--;
        if (is_invalid (ety.ty) || ety.ty->kind != Ty_array)
        {
            // TODO:  proper error here
            return e_invalid;
        }

        Semant_Exp sty = TransExp (context, var->u.subscript.exp);
        if (!is_int (sty))
        {
            // TODO:  proper error here
        }

        return Expression_New (
                   Tr_SubscriptVar (ety.exp, ety.ty, sty.exp, level == 0),
                   ety.ty->u.array.type);
    }
    }

    return e_invalid;
}

/*
 * TODO
 * Returns default initialization for a type
 * TODO move to translate
 */
static Semant_Exp TransDefaultValue (Tr_access access, Ty_ty type, int offset)
{
    type = GetActualType (type);
    switch (type->kind)
    {
    case Ty_int:
    {
        return Expression_New (Tr_Int (0), Ty_Int());
    }
    case Ty_array:
    {
        int size = type->u.array.size;
        int typesize = Ty_SizeOf (type->u.array.type);
        Tr_expList el = NULL;
        int o = offset;
        while (size--)
        {
            LIST_PUSH (el, TransDefaultValue (access, type->u.array.type, o).exp);
            o += typesize;
        }

        return Expression_New (Tr_ArrayExp (access, type, el, offset), type);
    }
    case Ty_record:
    {
        int o = offset;
        Tr_expList el = NULL;
        LIST_FOREACH (f, type->u.record)
        {
            LIST_PUSH (el, TransDefaultValue (access, f->ty, o).exp);
            o += Ty_SizeOf (f->ty);
        }

        return Expression_New (Tr_RecordExp (access, type, el, offset), type);
    }
    default:
    {
        assert (0);
    }
    }
}

static Semant_Exp TransExp (Semant_Context context, A_exp exp)
{
    Semant_Exp e_invalid = {Tr_Void(), Ty_Invalid()};
    Semant_Exp e_void = {Tr_Void(), Ty_Void()};

    switch (exp->kind)
    {
    case A_seqExp:
    {
        A_expList l = exp->u.seq;
        if (!l)
        {
            return e_void;
        }
        Semant_Exp r;
        Tr_exp seq = NULL;
        for (; l; l = l->tail)
        {
            r = TransExp (context, l->head);
            seq = Tr_Seq (seq, r.exp);
        }
        return Expression_New (seq, r.ty);
    }
    case A_varExp:
    {
        return TransVar (context, exp->u.var);
    }
    case A_asmExp:
    {
        const char * data = exp->u.assembly.data;
        U_stringList dst = exp->u.assembly.dst;
        U_stringList src = exp->u.assembly.src;

        return Expression_New (
                   Tr_Asm (
                       exp->u.assembly.code,
                       data == NULL ? NULL : Tr_String (context, data),
                       dst,
                       src),
                   Ty_Void());
    }
    case A_callExp:
    {
        Env_Entry fun = (Env_Entry)S_Look (context->venv, exp->u.call.func);
        if (!fun || fun->kind != Env_funEntry)
        {
            // TODO add proper error here
            // TODO Should it be void?
            // TODO How to make it clear that the object is not callable?
            return e_void;
        }

        Tr_expList el = NULL;
        {
            A_expList l = exp->u.call.args;
            Ty_tyList f = fun->u.fun.formals;
            for (; l || f; l = l->tail, f = f->tail)
            {
                if (!l || !f)
                {
                    // TODO spawn a proper error here
                    break;
                }

                if (TransExp (context, l->head).ty != f->head)
                {
                    // TODO spawn a proper error here
                }

                LIST_PUSH (el, TransExp (context, l->head).exp);
            }
        }

        return Expression_New (
                   Tr_Call (fun->u.fun.label, context->level, fun->u.fun.level, el),
                   fun->u.fun.result);
    }
    case A_nilExp:
    {
        return Expression_New (Tr_Nil(), Ty_Nil());
    }
    case A_intExp:
    {
        return Expression_New (Tr_Int (exp->u.intt), Ty_Int());
    }
    case A_stringExp:
    {
        return Expression_New (Tr_String (context, exp->u.stringg), Ty_String());
    }
    case A_opExp:
    {
        A_oper oper = exp->u.op.oper;

        Semant_Exp left = TransExp (context, exp->u.op.left);
        if (is_invalid (left.ty))
        {
            return left;
        }

        Semant_Exp right = TransExp (context, exp->u.op.right);
        if (is_invalid (right.ty))
        {
            return right;
        }

        if (oper == A_plusOp
                || oper == A_minusOp
                || oper == A_timesOp
                || oper == A_divideOp
                || oper == A_gtOp
                || oper == A_ltOp
                || oper == A_geOp
                || oper == A_leOp)
        {
            if (!is_int (left) || !is_int (right))
            {
                if (!is_int (left))
                {
                    ERROR_WRONG_TYPE (Ty_Int(), left.ty, exp->u.op.left->loc);
                }
                if (!is_int (right))
                {
                    ERROR_WRONG_TYPE (Ty_String(), right.ty, exp->u.op.right->loc);
                }

                return e_invalid;
            }

            return Expression_New (Tr_Op (oper, left.exp, right.exp, left.ty), Ty_Int());
        }
        else if (oper == A_eqOp
                 || oper == A_neqOp)
        {
            if (is_int (left) && !is_int (right))
            {
                ERROR_WRONG_TYPE (Ty_Int(), right.ty, exp->u.op.right->loc);
                return e_invalid;
            }
            else if (is_string (left) && !is_string (right))
            {
                ERROR_WRONG_TYPE (Ty_String(), right.ty, exp->u.op.right->loc);
                return e_invalid;
            }
            else if (same_type (left, right) && (is_int (left) || is_string (left)))
            {
                return Expression_New (Tr_Op (oper, left.exp, right.exp, left.ty), Ty_Int());
            }

            return e_invalid;

        }
    }
    case A_arrayExp:
    case A_recordExp:
    {
        static Tr_access access = NULL;
        static int level = 0;
        static int thisOffset = 0;

        Ty_ty ty = NULL;
        Tr_exp ex = NULL;

        int nextOffset = 0;

        if (level == 0)
        {
            /*
             * We do not the real type yet but we need to pass the base further down the tree
             * so we create a virtual access point(base), later it will be updated with the
             * real type information.
             */
            access = Tr_AllocVirtual (context->level);
        }

        if (exp->kind == A_arrayExp)
        {
            if (exp->u.array == NULL)
            {
                ERROR_MALFORMED_EXP (exp->loc, "Array expression cannot be empty");
                return e_invalid;
            }

            Tr_expList el = NULL;
            Ty_tyList tl = NULL;
            LIST_FOREACH (item, exp->u.array)
            {
                level++;
                Semant_Exp sexp = TransExp (context, item);
                level--;
                LIST_PUSH (el, sexp.exp);
                LIST_PUSH (tl, sexp.ty);

                int size = Ty_SizeOf (sexp.ty);
                thisOffset += size;
                nextOffset += size;
            }

            // returning to the current level offset
            thisOffset -= nextOffset;

            ty = tl->head;
            int size = 1;

            if (is_invalid (ty))
            {
                return e_invalid;
            }

            LIST_FOREACH (type, tl->tail)
            {
                size++;
                if (is_invalid (type))
                {
                    return e_invalid;
                }
                else if (type != ty)
                {
                    ERROR_MALFORMED_EXP (
                        exp->loc,
                        "Array expression expects values of the same type");
                    return e_invalid;
                }
            }

            ty = Ty_Array (ty, size);
            ex = Tr_ArrayExp (access, ty, el, thisOffset);
        }
        else
        {

            if (exp->u.record.name)
            {
                ty = GetActualType ((Ty_ty)S_Look (context->tenv, exp->u.record.name));
                if (!ty)
                {
                    ERROR_UNKNOWN_TYPE (exp->loc, exp->u.record.name);
                    return e_invalid;
                }
                if (is_invalid (ty))
                {
                    ERROR_INVALID_TYPE (exp->loc, exp->u.record.name);
                    return e_invalid;
                }
            }

            Tr_expList el = NULL;
            Ty_fieldList fl = NULL;
            bool valid = TRUE;
            /*
             * This is offset(as part of thisOffset value) is used by the lower record level if
             * exists.
             */
            LIST_FOREACH (f, exp->u.record.fields)
            {
                level++;
                Semant_Exp sexp = TransExp (context, f->exp);
                level--;
                LIST_PUSH (el, sexp.exp);
                if (is_invalid (sexp.ty))
                {
                    valid = FALSE;
                }

                int size = Ty_SizeOf (sexp.ty);
                thisOffset += size;
                nextOffset += size;

                // creating name: type list
                LIST_PUSH (fl, Ty_Field (f->name, sexp.ty));
            }

            // returning to the current level offset
            thisOffset -= nextOffset;

            if (!valid)
            {
                ty = Ty_Invalid();
            }
            else if (ty)
            {
                Tr_expList ael = NULL;

                // checking whether the named record type can be initialized with the anon struct

                // FIXME find fields that do not belong to the type
                nextOffset = 0;
                LIST_FOREACH (type_filed, ty->u.record)
                {
                    Tr_expList iel = el;
                    Tr_exp ie = NULL;

                    LIST_FOREACH (init_field, fl)
                    {
                        if (init_field->name == type_filed->name)
                        {
                            ie = iel->head;
                            break;
                        }

                        iel = iel->tail;
                    }

                    if (ie)
                    {
                        LIST_PUSH (ael, ie);
                    }
                    else
                    {
                        LIST_PUSH (ael, TransDefaultValue (
                                       access,
                                       type_filed->ty,
                                       thisOffset).exp);
                    }

                    int size = Ty_SizeOf (type_filed->ty);
                    thisOffset += size;
                    nextOffset += size;
                }

                // returning to the current level offset
                thisOffset -= nextOffset;

                el = ael;
            }
            // create anonymous type
            else if (!ty)
            {
                ty = Ty_Record (fl);
            }

            ex = Tr_RecordExp (access, ty, el, thisOffset);
        }


        if (level == 0)
        {
            /*
             * Now we know the type and the init tree is created using virtual access point,
             * we update the access
             */
            Tr_AllocMaterialize (access, context->level, ty, FALSE);
        }

        return Expression_New (ex, ty);
    }
    case A_assignExp:
    {
        Semant_Exp vty = TransVar (context, exp->u.assign.var);
        if (!is_truetype (vty.ty))
        {
            // TODO:  proper error here
        }

        Semant_Exp ety = TransExp (context, exp->u.assign.exp);
        if (!is_truetype (ety.ty))
        {
            // TODO:  proper error here
        }

        if (ety.ty != vty.ty)
        {
            // TODO:  proper error here
        }

        return Expression_New (Tr_Assign (vty.exp, ety.exp), Ty_Void());
    }
    case A_ifExp:
    {
        Semant_Exp test = TransExp (context, exp->u.iff.test);
        if (test.ty != Ty_Int())
        {
            ERROR_WRONG_TYPE (Ty_Int(), test.ty, exp->u.iff.test->loc);
        }

        Semant_Exp pos = e_void;
        if (exp->u.iff.tr)
        {
            pos = TransScope (context, exp->u.iff.tr);
            if (!is_truetype (pos.ty))
            {
                // TODO proper error handling here
                printf("fuck top\n");
                return e_void;
            }
        }

        Semant_Exp neg = e_void;
        if (exp->u.iff.fl)
        {
            neg = TransScope (context, exp->u.iff.fl);

            if (!is_truetype (neg.ty))
            {
                printf("fuck\n");
                // TODO proper error handling here
                return e_void;
            }
        }

        /* if (pos.ty != neg.ty) */
        /* { */
        /*     // TODO proper error handling here */
        /*     return e_void; */
        /* } */

        return Expression_New (Tr_If (test.exp, pos.exp, neg.exp), Ty_Void());
    }
    case A_whileExp:
    {
        Semant_Exp test = TransExp (context, exp->u.whilee.test);
        if (test.ty != Ty_Int())
        {
            ERROR_WRONG_TYPE (Ty_Int(), test.ty, exp->u.whilee.test->loc);
        }

        Temp_label label = context->breaker;
        Temp_label breaker = Temp_NewLabel();
        context->breaker = breaker;
        context->loopNesting++;

        Semant_Exp body = TransScope (context, exp->u.whilee.body);

        context->loopNesting--;
        context->breaker = label;

        return Expression_New (Tr_While (test.exp, body.exp, breaker), Ty_Void());
    }
    case A_forExp:
    {
        Semant_Exp expty_1 = TransExp (context, exp->u.forr.lo);
        if (!is_int (expty_1))
        {
            // TODO proper error handling here
        }

        Semant_Exp expty_2 = TransExp (context, exp->u.forr.hi);
        if (!is_int (expty_2))
        {
            // TODO proper error handling here
        }

        S_BeginScope (context->venv);
        S_Enter (
            context->venv,
            exp->u.forr.var,
            Env_VarEntryNew (
                Tr_Alloc (context->level, Ty_Int(), exp->u.forr.escape),
                Ty_Int()));

        Temp_label label = context->breaker;
        Temp_label breaker = Temp_NewLabel();
        context->breaker = breaker;
        context->loopNesting++;

        Semant_Exp body = TransScope (context, exp->u.forr.body);

        context->loopNesting--;
        context->breaker = label;

        S_EndScope (context->venv);

        return Expression_New (Tr_For (expty_1.exp, expty_2.exp, body.exp, breaker), Ty_Void());
    }
    case A_breakExp:
    {
        if (!context->loopNesting)
        {
            // TODO:  proper error here
        }
        return Expression_New (Tr_Break (context->breaker), Ty_Void());
    }
    }

    return e_invalid;
}

int Semant_Translate (Program_Module m)
{
    assert (m);

    //HMM shouldn't be done after semant analysis? what if there are errors?
    // e.g. it will SEGFLT if undefined var is used
    Escape_Find (m->ast);

    struct Semant_ContextType context;
    context.module = m;
    context.loopNesting = 0;

    Env_Init (&context);
    Tr_Init (&context);

    LIST_FOREACH (dec, m->ast) TransDec (&context, dec);

    return Vector_Size (&m->errors.semant);
}
