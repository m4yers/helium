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

#define ERROR(loc, code, format, ...)                                    \
    {                                                                    \
        Vector_PushBack(&context->module->errors.semant,                 \
                Error_New(loc, code, format, __VA_ARGS__));              \
    }                                                                    \

#define ERROR_WRONG_TYPE(loc, expected, actual)                          \
    ERROR(                                                               \
        loc,                                                             \
        3001,                                                            \
        "Expected '%s', got '%s'",                                       \
        expected->meta.name,                                             \
        actual->meta.name);                                              \

#define ERROR_MALFORMED_EXP(loc, text)                                   \
    ERROR(                                                               \
        loc,                                                             \
        3002,                                                            \
        "Malformed exprassion: %s",                                      \
        text);                                                           \

#define ERROR_INVALID_TYPE(loc, name)                                    \
    ERROR(                                                               \
        loc,                                                             \
        3003,                                                            \
        "Invalid type '%s'",                                             \
        S_Name(name));                                                   \

#define ERROR_UNKNOWN_TYPE(loc, name)                                    \
    ERROR(                                                               \
        loc,                                                             \
        3004,                                                            \
        "Unknown type '%s'",                                             \
        S_Name(name));                                                   \

#define ERROR_UNKNOWN_SYMBOL(loc, sym)                                   \
    ERROR(                                                               \
        loc,                                                             \
        3005,                                                            \
        "Unknown symbol '%s'",                                           \
        S_Name(sym));                                                    \

#define ERROR_INVALID_EXPRESSION(loc)                                    \
    ERROR(                                                               \
        loc,                                                             \
        3009,                                                            \
        "Invalid expression", "");

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
//
static bool is_int_type (Ty_ty ty)
{
    return ty == Ty_Int();
}

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
            ERROR_UNKNOWN_TYPE (&ty->loc, ty->u.name);
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
    A_loc loc = &dec->loc;

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
                ERROR_WRONG_TYPE (loc, ty, sexp.ty);
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
        struct A_decFn_t decFn = dec->u.function;

        // translate name
        Temp_label label = Tr_ScopedLabel (context->level, decFn.name->name);
        bool is_main = strcmp (label->name, "main") == 0;

        // translate return type
        Ty_ty rty = NULL;
        if (decFn.type)
        {
            rty =  TransTyp (context, decFn.type);
            if (is_main && !is_int_type (rty))
            {
                ERROR_WRONG_TYPE (loc, Ty_Int(), rty);
                // change to int and try parse the rest
                rty = Ty_Int();
            }
        }
        else if (is_main)
        {
            rty = Ty_Int();
        }
        else
        {
            rty = Ty_Auto();
        }

        // translate formal parameters
        S_symbolList names = NULL;
        Ty_tyList types = NULL;
        U_boolList escapes = NULL;
        LIST_FOREACH (f, decFn.params)
        {
            Ty_ty fty = TransTyp (context, f->type);

            LIST_PUSH (names, f->name);
            LIST_PUSH (types, fty);
            LIST_PUSH (escapes, f->escape);
        }

        // create new frame
        Tr_level level = Tr_NewLevel (context->level, label, escapes);

        // create environment function entry
        Env_Entry entry = Env_FunEntryNew (level, label, names, types, rty);
        S_Enter (context->venv, decFn.name, entry);

        // create new scope for the body
        S_BeginScope (context->venv);
        S_BeginScope (context->tenv);

        // add each formal parameter binding to the newly entered scope
        {
            A_fieldList f = decFn.params;
            Tr_accessList al = Tr_Formals (entry->u.fun.level);
            Ty_tyList t = types;
            for (; f; f = f->tail, t = t->tail, al = al->tail)
            {
                S_Enter (
                    context->venv,
                    f->head->name,
                    Env_VarEntryNew (al->head, t->head));
            }
        }

        // add implicit return
        A_stm last;
        LIST_BACK (decFn.scope->list, last);
        if (last->kind == A_stmExp && last->u.exp->kind != A_retExp)
        {
            A_exp e = last->u.exp;
            last->u.exp = A_RetExp (&e->loc, e);
        }
        /*
         * main function is treated specially because its job is to return program's execution
         * status, where 0 means success and any positive value is treated as an error. Thus if
         * the last statement in the main function is some sort of declaration we add implicit
         * ret 0 statement to the main's body end.
         */
        else if (last->kind == A_stmDec && is_main)
        {
            LIST_PUSH (decFn.scope->list, A_StmExp (A_RetExp (loc, A_IntExp (loc, 0))));
        }

        // save current frame before processing the body
        Tr_level current = context->level;

        // set the new frame as current so the body can use it
        context->level = level;

        // translate body
        Semant_Exp sexp = TransScope (context, decFn.scope);
        Tr_ProcEntryExit (context, context->level, sexp.exp);

        // restore frame
        context->level = current;

        // final type checks
        if (is_auto (rty))
        {
            entry->u.fun.result = sexp.ty;
        }
        else if (is_main && !is_int (sexp))
        {
            ERROR_WRONG_TYPE (&last->u.exp->loc, Ty_Int(), sexp.ty);
        }
        else if (!is_unknown (rty) && !is_invalid (rty) && rty != sexp.ty)
        {
            ERROR_WRONG_TYPE (&last->u.exp->loc, rty, sexp.ty);
        }

        // restore scope
        S_EndScope (context->venv);
        S_EndScope (context->tenv);

        return Tr_Void();
    }
    }
}

static Semant_Exp TransVar (Semant_Context context, A_var var)
{
    A_loc loc = &var->loc;
    Semant_Exp e_invalid = {Tr_Void(), Ty_Invalid()};

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
        if (!e)
        {
            ERROR_UNKNOWN_SYMBOL(&var->loc, var->u.simple)
            return e_invalid;
        }
        else if (e->kind == Env_varEntry)
        {
            return Expression_New (
                       Tr_SimpleVar (e->u.var.access, context->level),
                       GetActualType (e->u.var.ty));
        }
        else
        {
            ERROR_UNKNOWN_SYMBOL (loc, var->u.simple);
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
    A_loc loc = &exp->loc;
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
        LIST_FOREACH (e, l)
        {
            r = TransExp (context, e);
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
    case A_retExp:
    {
        Semant_Exp sexp = TransExp (context, exp->u.ret);
        return Expression_New (Tr_Ret (context->level, sexp.exp), sexp.ty);
    }
    case A_callExp:
    {
        struct A_callExp_t callExp = exp->u.call;

        // query environment for the function name
        Env_Entry entry = (Env_Entry)S_Look (context->venv, callExp.func);
        if (!entry)
        {
            ERROR_UNKNOWN_SYMBOL (loc, callExp.func);
            return e_void;
        }
        else if (entry->kind != Env_funEntry)
        {
            ERROR (loc, 3006, "Symbol '%s' is not callable", S_Name (callExp.func));
            return e_void;
        }

        struct Env_funEntry_t fnEntry = entry->u.fun;

        // check arguments types with formal type list and if everything is ok translate
        Tr_expList tral = NULL;
        A_expList al = callExp.args;
        S_symbolList names = fnEntry.names;
        LIST_FOREACH (t, fnEntry.types)
        {
            if (!al)
            {
                ERROR (loc, 3007, "Missed a call argument '%s' of '%s'",
                       names->head->name,
                       t->meta.name);
            }
            else
            {
                Semant_Exp sexp = TransExp (context, al->head);

                if (sexp.ty != t)
                {
                    ERROR_WRONG_TYPE (&al->head->loc, t, sexp.ty);
                }

                LIST_PUSH (tral, sexp.exp);
            }

            names = LIST_NEXT (names);
            al = LIST_NEXT (al);
        }

        // check for superfluous arguments
        LIST_FOREACH (a, al)
        {
            ERROR (&a->loc, 3008, "Unexpected argument", "");
        }

        return Expression_New (Tr_Call (
                                   fnEntry.label,
                                   context->level,
                                   fnEntry.level,
                                   tral),
                               fnEntry.result);
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
        struct A_opExp_t opExp = exp->u.op;

        A_oper oper = opExp.oper;

        Semant_Exp left = TransExp (context, opExp.left);
        Semant_Exp right = TransExp (context, opExp.right);

        // do type checks
        if (oper == A_plusOp
                || oper == A_minusOp
                || oper == A_timesOp
                || oper == A_divideOp
                || oper == A_gtOp
                || oper == A_ltOp
                || oper == A_geOp
                || oper == A_leOp
                || oper == A_eqOp
                || oper == A_neqOp)
        {
            /*
             * We do type recovery here to parse the unit further assuming these operations are
             * applicable for int only.
             */
            if (!is_int (left) || !is_int (right))
            {
                if (!is_int (left))
                {
                    ERROR_WRONG_TYPE (&opExp.left->loc, Ty_Int(), left.ty);
                    left.ty = Ty_Int();
                }
                if (!is_int (right))
                {
                    ERROR_WRONG_TYPE (&opExp.right->loc, Ty_Int(), right.ty);
                    right.ty = Ty_Int();
                }
            }

        }
        return Expression_New (Tr_Op (oper, left.exp, right.exp, left.ty), left.ty);
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
                ERROR_MALFORMED_EXP (&exp->loc, "Array expression cannot be empty");
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
                        &exp->loc,
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
                    ERROR_UNKNOWN_TYPE (&exp->loc, exp->u.record.name);
                    return e_invalid;
                }
                if (is_invalid (ty))
                {
                    ERROR_INVALID_TYPE (&exp->loc, exp->u.record.name);
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
        struct A_assignExp_t assignExp = exp->u.assign;

        Semant_Exp lexp = TransVar (context, assignExp.var);
        Semant_Exp rexp = TransExp (context, assignExp.exp);

        // no way to recover from this
        if (is_invalid (lexp.ty) && is_invalid (rexp.ty))
        {
            ERROR_INVALID_EXPRESSION (&exp->loc)
            return e_invalid;
        }
        /*
         * partially valid expression, here we try to recover by mutually assigning correct
         * types in hope it won't break further analysis. We do not push any errors because they
         * were already fired by the lexp and rexp translations calls.
         */
        else if (is_invalid (lexp.ty))
        {
            lexp.ty = rexp.ty;
        }
        else if (is_invalid (rexp.ty))
        {
            rexp.ty = lexp.ty;
        }
        // correct but unmatched types, fix the type of the right expression
        else if (lexp.ty != rexp.ty)
        {
            ERROR_WRONG_TYPE (&assignExp.exp->loc, lexp.ty, rexp.ty)
            rexp.ty = lexp.ty;
        }

        return Expression_New (Tr_Assign (lexp.exp, rexp.exp), rexp.ty);
    }
    case A_ifExp:
    {
        struct A_ifExp_t ifExp = exp->u.iff;

        Semant_Exp texp = TransExp (context, ifExp.test);

        // currenlty Helium does not support bool
        if (texp.ty != Ty_Int())
        {
            ERROR_WRONG_TYPE (&ifExp.test->loc, Ty_Int(), texp.ty);
        }

        Semant_Exp pexp = e_void;
        if (ifExp.tr)
        {
            pexp = TransScope (context, ifExp.tr);
        }

        Semant_Exp nexp = e_void;
        if (ifExp.fl)
        {
            nexp = TransScope (context, ifExp.fl);
        }

        return Expression_New (Tr_If (texp.exp, pexp.exp, nexp.exp), Ty_Void());
    }
    case A_whileExp:
    {
        Semant_Exp test = TransExp (context, exp->u.whilee.test);
        if (test.ty != Ty_Int())
        {
            ERROR_WRONG_TYPE (&exp->u.whilee.test->loc, Ty_Int(), test.ty);
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
    default:
    {
        assert (0);
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
