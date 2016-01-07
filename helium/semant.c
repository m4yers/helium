#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include "ext/table.h"
#include "ext/list.h"

#include "types.h"
#include "semant.h"
#include "semant_mips.h"
#include "ast.h"
#include "types.h"
#include "symbol.h"
#include "escape.h"
#include "error.h"
#include "env.h"

#define ERROR(loc, code, format, ...)                                    \
    {                                                                    \
        printf ("error line: %d\n", __LINE__);\
        Vector_PushBack(&context->module->errors.semant,                 \
                Error_New(loc, code, format, __VA_ARGS__));              \
    }                                                                    \

#define ERROR_UNEXPECTED_TYPE(loc, expected, actual)                     \
    ERROR(                                                               \
        loc,                                                             \
        3001,                                                            \
        "Expected '%s', got '%s'",                                       \
        GetQTypeName(expected, NULL)->data,                              \
        GetQTypeName(actual, NULL)->data);                               \

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

#define ERROR_EXPECTED_CONSTANT_EXPRESSION(loc)                          \
    ERROR(                                                               \
        loc,                                                             \
        3011,                                                            \
        "Expected constat expression", "");

#define ERROR_SYMBOL_EXISTS(loc, sym)                                    \
    ERROR(                                                               \
        loc,                                                             \
        3014,                                                            \
        "Symbol '%s' already exist",                                     \
        S_Name(sym));                                                    \

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

static bool is_int_type (Ty_ty ty)
{
    return ty == Ty_Int();
}

static bool is_int (Semant_Exp exp)
{
    return exp.ty == Ty_Int() || GetActualType (exp.ty) == Ty_Int();
}

static bool is_str (Semant_Exp exp)
{
    return exp.ty == Ty_Str();
}

static bool same_type (Semant_Exp a, Semant_Exp b)
{
    return a.ty == b.ty;
}

static bool campatible_types (Ty_ty a, Ty_ty b)
{
    a = GetActualType (a);
    b = GetActualType (b);

    return a == b;
}

static Semant_Exp Expression_New (Tr_exp exp, Ty_ty ty)
{
    Semant_Exp r;
    r.exp = exp;
    r.ty = ty;
    return r;
}

/*
 * The method retrieves fully qualified type name and tries to retrieve its instance from the
 * environment context, if it is there the instance will be resturned as result, if not, the passed
 * type instance will be used to create new type entry and the same instance will be used as result.
 */
static Ty_ty GetOrCreateTypeEntry (Semant_Context context, Ty_ty ty)
{
    struct String_t ty_id = String ("");
    GetQTypeName (ty, &ty_id);
    S_symbol symbol = S_Symbol (ty_id.data);

    Ty_ty ety = (Ty_ty)S_Look (context->tenv, symbol);

    if (!ety)
    {
        S_Enter (context->tenv, symbol, ty);;
        ety = ty;
    }

    return ety;
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

    switch (ty->kind)
    {
    case A_nameTy:
    {
        Ty_ty type = (Ty_ty)S_Look (context->tenv, ty->u.name);
        if (!type)
        {
            ERROR_UNKNOWN_TYPE (&ty->loc, ty->u.name);
            return Ty_Invalid();
        }

        return type;
    }
    case A_pointerTy:
    {
        Ty_ty type = TransTyp (context, ty->u.pointer);
        if (is_invalid (type))
        {
            type = Ty_Int();
        }

        return GetOrCreateTypeEntry (context, Ty_Pointer (type));
    }
    case A_arrayTy:
    {
        struct A_arrayTy_t arrayTy = ty->u.array;

        /*
         * If the type of array is not a valid type we change it to int and proceed, this will
         * not bring ripple effect to error generation. The user will be only notified by the
         * error produced by A_nameTy look-up
         */
        Ty_ty type = TransTyp (context, arrayTy.type);
        if (is_invalid (type))
        {
            type = Ty_Int();
        }

        /*
         * As with the type the type we do not break the translation process and replace an invalid
         * size expression with int(1), spawn error and proceed.
         */
        int size;
        if (arrayTy.size->kind != A_intExp)
        {
            ERROR_EXPECTED_CONSTANT_EXPRESSION (&arrayTy.size->loc)
            size = 1;
        }
        else
        {
            size = arrayTy.size->u.intt;
        }

        return GetOrCreateTypeEntry (context, Ty_Array (type, size));
    }
    case A_recordTy:
    {
        Ty_fieldList flist = NULL;
        LIST_FOREACH (field, ty->u.record)
        {
            // check for field repetition
            LIST_FOREACH (field_s, ty->u.record)
            {
                if (field_s == field)
                {
                    break;
                }
                else if (field_s->name == field->name)
                {
                    ERROR (
                        &field_s->loc,
                        3002,
                        "Redefinition of field '%s'",
                        field_s->name->name);
                }
            }

            /*
             * Same as for the array translation we do not break the process but simply fallback
             * to the default type int
             */
            Ty_ty type = TransTyp (context, field->type);
            if (is_invalid (type))
            {
                type = Ty_Int();
            }

            LIST_PUSH (flist, Ty_Field (field->name, type))
        }

        return Ty_Record (flist);
    }
    default:
    {
        assert (0);
    }
    }
}

// TODO you need to do it in two passes: 1 - names, 2 - definitions
static Tr_exp TransDec (Semant_Context context, A_dec dec)
{
    A_loc loc = &dec->loc;

    switch (dec->kind)
    {
    // TODO when forward declaration will be available check for cycle typedefs
    case A_typeDec:
    {
        struct A_decType_t decType = dec->u.type;
        Ty_ty type = TransTyp (context, decType.type);

        // fallback to int if type is invalid
        if (is_invalid (type))
        {
            type = Ty_Int();
        }

        // check for typedef duplicate
        Ty_ty def = (Ty_ty)S_Look (context->tenv, decType.name);

        if (def)
        {
            ERROR (
                &dec->loc,
                3014,
                "Redefinition of type '%s'",
                decType.name->name);
        }
        else
        {
            S_Enter (context->tenv, decType.name, Ty_Name (decType.name, type));;
        }
        return Tr_Void();
    }
    case A_varDec:
    {
        struct A_decVar_t decVar = dec->u.var;

        Ty_ty dty = NULL;
        if (decVar.type)
        {
            dty = TransTyp (context, decVar.type);
            /*
             * The type error was spawn by TransTyp already, this fallback will require type
             * inferring based on init expression if it exists, otherwise the translation will
             * return Tr_Void, in this case the variable won't be declared and all reference to
             * it will be illegal.
             */
            if (is_invalid (dty))
            {
                dty = NULL;
            }
            else
            {
                dty = GetActualType (dty);
            }
        }

        Ty_ty ity = NULL;
        Tr_exp iexp = NULL;
        if (decVar.init)
        {
            Semant_Exp sexp = TransExp (context, decVar.init);
            /*
             * If the initialization expression is invalid we simply drop it, the actual error was
             * spawn by TransExp scope.
             */
            if (!is_invalid (sexp.ty))
            {
                iexp = sexp.exp;
                ity = sexp.ty;
            }
        }

        Tr_access access = NULL;
        if (iexp)
        {
            Ty_ty aity = GetActualType (ity);
            /*
             * There are three possible init scenarios:
             *
             *  1. The variable is assigned a simple value, like int or pointer, in this case we
             *     do simple temp-to-temp move.
             *
             *  2. The variable is initialized with an array or record expression. In this case
             *     a space is allocated on stack by processing this init exp, in this case we just
             *     need to move handle temp to the new var temp and regalloc will coalesce these
             *     two temps.
             *
             *  3. The variable is assigned another variable that was initialized with an array or
             *     record expression(basically any var that is handle in nature). In this case we
             *     need to allocate the same array on stack and do deep copy of the var.
             *
             *  In case of 1 or 2 we just do temp-to-temp move, in the third case we do deep copy.
             */

            /*
             * If the rhs is not an handle expression but yields handle type we need to copy it, so:
             *
             * let a = Point{}; // handle only copy
             * let b = a;       // handle and frame array copy
             */
            bool copy = decVar.init->kind != A_arrayExp
                        && decVar.init->kind != A_recordExp
                        && aity->meta.is_handle;

            /*
             * Handle types are based on implicit pointer, upon taking the address of var of
             * this kind the address is copied from the handle, so no need to push it to the
             * stack.
             *
             * NOTE escape analysis cannot fully handle this because if type inferring.
             */
            bool escape = aity->meta.is_handle ? FALSE : decVar.escape;

            /*
             * The difference here is in the type size, if we copy the stack array we allocate full
             * size, if we just move handle we allocate word-size temp.
             */
            if (copy)
            {
                access = Tr_Alloc (context->level, aity, decVar.var, escape);
                Tr_exp left = Tr_SimpleVar (access, context->level, TRUE);
                iexp = Tr_Memcpy (left, iexp, Ty_SizeOf (aity) / F_wordSize);
            }
            else
            {
                access = Tr_Alloc (context->level, Ty_Int(), decVar.var, escape);
                iexp = Tr_Assign (Tr_SimpleVar (access, context->level, TRUE), iexp);
            }

            /*
             * If provided initialization expression is not of the variable type, spawn an error
             * and drop initialization completely.
             */
            if (dty && dty != ity && GetActualType (dty) != aity)
            {
                ERROR_UNEXPECTED_TYPE (loc, dty, ity);
                iexp = Tr_Void();
            }
            // initializing a non-typed variable with nil is a bad idea
            else if (!dty && ity == Ty_Nil())
            {
                ERROR (&dec->loc,
                       3013,
                       "Invalide declaration, you cannot init a non-typed variable '%s' with nil",
                       decVar.var->name);
            }
            // Type inferring
            else
            {
                dty = ity;
            }

        }
        // We have only type supplied so no initialization is made.
        else if (dty)
        {
            // same as above
            bool escape = GetActualType (dty)->meta.is_handle ? FALSE : decVar.escape;
            dty = GetActualType (dty);
            access = Tr_Alloc (context->level, GetActualType (dty), decVar.var, escape);
            iexp = Tr_Void();
        }
        else
        {
            return Tr_Void();
        }

        /*
         * check if the variable with the same name exist in the current scope; this is different
         * from type scope checks because we check the top most scope, meaning innner code blocks
         * can shadow outer blocks variable declarations.
         */
        Env_Entry e = (Env_Entry)S_LookTop (context->venv, decVar.var);

        if (e && e->level == context->level)
        {
            ERROR (
                &dec->loc,
                3014,
                "Redefinition of '%s'",
                decVar.var->name);
        }
        else
        {
            S_Enter (context->venv, decVar.var, Env_VarEntryNew (context->level, access, dty));
        }

        return iexp;
    }
    // TODO functions without any exp?
    // TODO general and main fn
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
                ERROR_UNEXPECTED_TYPE (loc, Ty_Int(), rty);
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


        /*
         * check if the function with the same name exist in the current scope; this is different
         * from type scope checks because we check the top most scope, meaning innner code blocks
         * can shadow outer blocks function declarations.
         */
        Env_Entry entry = (Env_Entry)S_LookTop (context->venv, decFn.name);

        if (entry && entry->level == context->level)
        {
            ERROR (
                &dec->loc,
                3014,
                "Redefinition of '%s'",
                decFn.name->name);
        }
        // else is not an option here because we need continue parsing

        entry = Env_FunEntryNew (context->level, level, label, names, types, rty);
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
                    Env_VarEntryNew (context->level, al->head, t->head));
            }
        }

        // add implicit return
        A_stm last = LIST_BACK (decFn.scope->list);
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
            ERROR_UNEXPECTED_TYPE (&last->u.exp->loc, Ty_Int(), sexp.ty);
        }
        else if (!is_invalid (rty) && !is_invalid (rty) && rty != sexp.ty)
        {
            ERROR_UNEXPECTED_TYPE (&last->u.exp->loc, rty, sexp.ty);
        }

        // restore scope
        S_EndScope (context->venv);
        S_EndScope (context->tenv);

        return Tr_Void();
    }
    default:
    {
        assert (0);
    }
    }
}

// FIXME deref thing is very unclear, i need a better approach
// mb trace tree path via stack? -> meaning usage context
static Semant_Exp TransVar (Semant_Context context, A_var var, bool deref)
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
            ERROR_UNKNOWN_SYMBOL (&var->loc, var->u.simple)
            return e_invalid;
        }
        else if (e->kind == Env_varEntry)
        {
            Ty_ty vty = e->u.var.ty;
            Ty_ty avty = GetActualType (vty);
            Ty_ty type = deref
                         ? vty
                         : GetOrCreateTypeEntry (context, Ty_Pointer (e->u.var.ty));
            // SHIT order here matters
            // in case of handle i need the address which is the same routine as for non-address-of
            // approach it is just handled differently in the context above, FUCK!!!
            deref = avty->meta.is_handle ? TRUE : deref;
            Tr_exp exp = Tr_SimpleVar (e->u.var.access, context->level, deref);

            return Expression_New (exp, type);
        }
        else
        {
            ERROR_UNKNOWN_SYMBOL (loc, var->u.simple);
            return e_invalid;
        }
    }
    case A_fieldVar:
    {
        struct A_varField_t varField = var->u.field;

        level++;
        Semant_Exp vexp = TransVar (context, varField.var, deref);
        level--;

        if (is_invalid (vexp.ty))
        {
            return e_invalid;
        }

        /*
         * Jumps(or dereference) for records is a bit tricky, first indirection level is basically
         * the same as normal field access:
         *
         *  def Point = {x: int, y: int}
         *  a = Point{};
         *  b = &a;
         *
         * Internal representation for a and b is the same the only difference is pointer type of b,
         * which just accents the records implementation as stack array plus handle(which is pointer).
         *
         * Thus on very first indirection level we MUST NOT derefence the value.
         */

        int jumps = varField.jumps;
        bool first = TRUE;

        while (jumps--)
        {
            if (vexp.ty->kind != Ty_pointer)
            {
                ERROR_UNEXPECTED_TYPE (&var->loc, Ty_Pointer (vexp.ty), vexp.ty);
                return e_invalid;
            }

            if (!first)
            {
                vexp.exp = Tr_DerefExp (vexp.exp);
            }
            vexp.ty = vexp.ty->u.pointer;
            first = FALSE;
        }

        vexp.ty = GetActualType (vexp.ty);
        if (vexp.ty->kind != Ty_record)
        {
            ERROR (
                &var->loc,
                3012,
                "Cannot access field '%s' of non record instance of type '%s'",
                varField.sym->name,
                vexp.ty->meta.name);

            return e_invalid;
        }

        LIST_FOREACH (f, vexp.ty->u.record)
        {
            if (f->name == varField.sym)
            {
                Ty_ty ty = GetActualType (f->ty);
                /*
                 * All primitive and handle types are returned by value, but in case of a handle
                 * type we need not to fetch the value and do return base+offset so the following
                 * parser will do memory copy starting from this location based on the type size.
                 *
                 * Specifically this line means we have returned to the root parse level of the
                 * current instance so the whole offset has been computed in form of base+offset
                 * and that the type of the instance we are parsing is primitive(single word size).
                 */
                deref = deref && level == 0 && !ty->meta.is_handle;
                Tr_exp exp = Tr_FieldVar (vexp.exp, vexp.ty, f->name, deref);
                return Expression_New (exp, ty);
            }
        }

        ERROR (
            &var->loc,
            3012,
            "Record type '%s' does not contain field '%s'",
            vexp.ty->meta.name,
            varField.sym->name);

        return e_invalid;
    }
    case A_subscriptVar:
    {
        struct A_varSubscript_t varSubscript = var->u.subscript;

        level++;
        Semant_Exp vexp = TransVar (context, varSubscript.var, deref);
        level--;

        if (is_invalid (vexp.ty))
        {
            return e_invalid;
        }
        else if (vexp.ty->kind != Ty_array)
        {
            ERROR (
                &var->loc,
                3012,
                "Cannot subscript non array instance of type '%s'",
                vexp.ty->meta.name);

            return e_invalid;
        }

        Ty_ty ty = GetActualType (vexp.ty->u.array.type);

        Semant_Exp sexp = TransExp (context, var->u.subscript.exp);
        /*
         * Essentially, array subscript expects an Int expression, if the subscript is not an Int
         * we simply spawn an error and try to recover by using a correct Int(0) translation.
         */
        if (!is_int (sexp))
        {
            ERROR_UNEXPECTED_TYPE (&varSubscript.exp->loc, Ty_Int(), sexp.ty);
            sexp.exp = Tr_Int (0);
            sexp.ty = Ty_Int();
        }

        /*
         * All primitive and handle types are returned by value, but in case of a handle type we need
         * not to fetch the value and do return base+offset so the following parser will do memory
         * copy starting from this location based on the type size.
         *
         * Specifically this line means we have returned to the root parse level of the current
         * instance so the whole offset has been computed in form of base+offset and that the type
         * of the instance we are parsing is primitive(single word size).
         */
        deref = deref && level == 0 && !ty->meta.is_handle;

        Tr_exp exp = Tr_SubscriptVar (vexp.exp, vexp.ty, sexp.exp, deref);
        return Expression_New (exp, ty);
    }
    default:
    {
        assert (0);
    }
    }
}

/*
 * Returns default initialization for a type
 */
static Semant_Exp TransDefaultValue (Tr_exp base, Ty_ty type, int offset)
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
            LIST_PUSH (el, TransDefaultValue (base, type->u.array.type, o).exp);
            o += typesize;
        }

        return Expression_New (Tr_ArrayExp (base, type, el, offset), type);
    }
    case Ty_record:
    {
        int o = offset;
        Tr_expList el = NULL;
        LIST_FOREACH (f, type->u.record)
        {
            Semant_Exp sexp = TransDefaultValue (base, f->ty, o);
            LIST_PUSH (el, sexp.exp);
            o += Ty_SizeOf (f->ty);
        }

        return Expression_New (Tr_RecordExp (base, type, el, offset), type);
    }
    default:
    {
        assert (0);
    }
    }
}

static Semant_Exp TransHandleExp (Semant_Context context, A_exp exp, Tr_exp b)
{
    A_loc loc = &exp->loc;
    Semant_Exp e_invalid = {Tr_Void(), Ty_Invalid()};

    Tr_access access = NULL;

    bool vaccess = FALSE;

    // FIXME do not use static
    static Tr_exp base;
    static int level = 0;
    static int thisOffset = 0;

    Ty_ty ty = NULL;
    Tr_exp ex = NULL;

    /*
     * This is offset(as part of thisOffset value) is used by the lower record level if
     * exists.
     */
    int nextOffset = 0;

    if (level == 0)
    {
        if (b)
        {
            base = b;
        }
        else
        {
            /*
             * We do not the real type yet but we need to pass the base further down the tree
             * so we create a virtual access point(base), later it will be updated with the
             * real type information.
             */
            access = Tr_AllocVirtual (context->level, NULL);
            vaccess = TRUE;

            /*
             * We need the address inside this access so we do deref
             */
            base = Tr_SimpleVar (access, context->level, TRUE);
        }

    }

    if (exp->kind == A_arrayExp)
    {
        if (exp->u.array == NULL)
        {
            ERROR_MALFORMED_EXP (loc, "Array expression cannot be empty");
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

        /*
         * Array expression always yields anonymous array type. It is done for consistency
         * sake, so the equal expressions will get the same(instance address wise) type.
         */
        ty = GetOrCreateTypeEntry (context, Ty_Array (ty, size));

        ex = Tr_ArrayExp (base, ty, el, thisOffset);
    }
    else
    {
        // Check whether ty record type is named, if so validate it.
        if (exp->u.record.name)
        {
            ty = (Ty_ty)S_Look (context->tenv, exp->u.record.name);
            if (!ty)
            {
                ERROR_UNKNOWN_TYPE (&exp->loc, exp->u.record.name);
                return e_invalid;
            }
            else if (is_invalid (ty))
            {
                ERROR_INVALID_TYPE (&exp->loc, exp->u.record.name);
                return e_invalid;
            }
            else if (GetActualType (ty)->kind != Ty_record)
            {
                ERROR (&exp->loc, 3002, "Type '%s' is not a record type", ty->meta.name);
                return e_invalid;
            }
        }

        // traverse expression in curly braces.
        Tr_expList el = NULL;
        Ty_fieldList fl = NULL;
        bool valid = TRUE;
        LIST_FOREACH (exp_field, exp->u.record.fields)
        {
            // check for field repetion
            LIST_FOREACH (exp_field_s, exp->u.record.fields)
            {
                if (exp_field_s == exp_field)
                {
                    break;
                }
                else if (exp_field_s->name == exp_field->name)
                {
                    ERROR (
                        &exp_field->loc,
                        3002,
                        "Redefinition of field '%s'",
                        exp_field->name->name);
                    valid = FALSE;
                }
            }

            level++;
            Semant_Exp sexp = TransExp (context, exp_field->exp);
            level--;

            if (is_invalid (sexp.ty))
            {
                valid = FALSE;
            }

            /*
             * If we have a specified type we need to do these things:
             *  - check for type matching
             *  - check for field belonging
             */
            if (valid && ty)
            {
                Ty_field ty_field = NULL;
                LIST_FOREACH (type_field, GetActualType (ty)->u.record)
                {
                    if (type_field->name == exp_field->name)
                    {
                        ty_field = type_field;
                        break;
                    }
                }

                // init expression contains a field that does not belong to the specified type
                if (!ty_field)
                {
                    ERROR (
                        &exp_field->loc,
                        3002,
                        "Unknown to type '%s' field '%s' of type '%s'",
                        ty->meta.name,
                        exp_field->name->name,
                        sexp.ty->meta.name);

                    valid = FALSE;
                }
                // types of init expression and type field mismatch
                else if (GetActualType (ty_field->ty) != GetActualType (sexp.ty))
                {
                    ERROR_UNEXPECTED_TYPE (&exp_field->loc, ty_field->ty, sexp.ty);
                    valid = FALSE;
                }
            }

            // creating init expression list
            LIST_PUSH (el, sexp.exp);

            // creating name ~ type list
            LIST_PUSH (fl, Ty_Field (exp_field->name, sexp.ty));

            int size = Ty_SizeOf (sexp.ty);
            thisOffset += size;
            nextOffset += size;
        }

        // returning to the current level offset
        thisOffset -= nextOffset;

        // field traversal yield invalid expression
        if (!valid)
        {
            ty = Ty_Invalid();
        }
        else if (ty)
        {
            // We need to rearrange the init expression list in type order and fill the gaps
            Tr_expList ael = NULL;
            nextOffset = 0;
            LIST_FOREACH (type_field, GetActualType (ty)->u.record)
            {
                Tr_expList iel = el;
                Tr_exp ie = NULL;
                Ty_ty tyty = type_field->ty;

                LIST_FOREACH (init_field, fl)
                {
                    if (init_field->name == type_field->name)
                    {
                        ie = iel->head;
                        break;
                    }

                    iel = iel->tail;
                }

                // found the proper name in type that matches init expression
                if (ie)
                {
                    /* printf ("init value for '%s'\n", type_field->name->name); */
                    LIST_PUSH (ael, ie);
                }
                // did not found any match, use default value
                else
                {
                    /* printf ("default value for '%s'\n", type_field->name->name); */
                    LIST_PUSH (ael, TransDefaultValue (base, tyty, thisOffset).exp);
                }

                int size = Ty_SizeOf (tyty);
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
            // TODO store anonymous records
            // HMM do i actually need to store anon records for module linking? anyway not now
            ty = Ty_Record (fl);
        }

        ex = Tr_RecordExp (base, GetActualType (ty), el, thisOffset);
    }


    if (!is_invalid (ty) && level == 0 && vaccess)
    {
        /*
         * Now we know the type and the init tree is created using virtual access point,
         * we update the access
         */
        Tr_AllocMaterialize (access, context->level, ty, FALSE);

        base = NULL;
    }

    return Expression_New (ex, ty);

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
        return TransVar (context, exp->u.var, TRUE);
    }
    case A_addressOf:
    {
        return TransVar (context, exp->u.addressOf, FALSE);
    }
    case A_valueAt:
    {
        Semant_Exp sexp = TransExp (context, exp->u.valueAt);

        // no point to try recrover here
        if (is_invalid (sexp.ty))
        {
            return sexp;
        }

        Ty_ty aty = GetActualType (sexp.ty);

        if (!aty->meta.is_pointer)
        {
            ERROR_UNEXPECTED_TYPE (&exp->loc, Ty_Pointer (sexp.ty), sexp.ty);
            sexp.ty = Ty_Invalid();
            return sexp;
        }
        /*
         * Handle internally passed as address and using value or pointer is based on context of
         * the expression, so we just change the type and return whatever is there.
         */
        else if (GetActualType (aty->u.pointer)->meta.is_handle)
        {
            printf ("handle deref\n");
            sexp.ty = sexp.ty->u.pointer;
        }
        /*
         * Primitive values on the other hand must be dereferenced explicitly
         */
        else
        {
            printf ("non handle deref\n");
            sexp.ty = sexp.ty->u.pointer;
            sexp.exp = Tr_DerefExp (sexp.exp);
        }
        return sexp;
    }
    case A_typeCastExp:
    {
        Ty_ty cty = TransTyp (context, exp->u.typeCast.type);
        Ty_ty acty = GetActualType (cty);
        Semant_Exp sexp = TransExp (context, exp->u.typeCast.exp);
        Ty_ty ety = sexp.ty;
        Ty_ty aety = GetActualType (ety);

        /*
         * If types match we do nothing
         */
        if (ety == cty)
        {
            return sexp;
        }

        /*
         * Types do not match but both are handle type so we just verify the sizes match and replace
         * current expression type with requested.
         */
        else if (acty->meta.is_handle && aety->meta.is_handle)
        {
            size_t size_cty = Ty_SizeOf (cty);
            size_t size_ety = Ty_SizeOf (ety);

            if (size_ety != size_cty)
            {
                ERROR (
                    &exp->loc, 3015,
                    "Cannot cast '%s' to '%s'",
                    GetQTypeName (ety, NULL)->data,
                    GetQTypeName (cty, NULL)->data);
            }

            sexp.ty = cty;
            return sexp;
        }
        /*
         * Pointers and handles are treated interchangeably if underlying pointer type is the same
         * as handle
         */
        else if ((acty->meta.is_pointer && aety->meta.is_pointer)
                 || (acty->meta.is_pointer && aety->meta.is_handle)
                 || (acty->meta.is_handle && aety->meta.is_pointer))
        {
            double size_cty = cty->meta.is_pointer ? Ty_SizeOf (cty->u.pointer) : Ty_SizeOf (cty);
            double size_ety = ety->meta.is_pointer ? Ty_SizeOf (ety->u.pointer) : Ty_SizeOf (ety);

            /*
             * Ideally this would divide evenly in range [1;N]. If this is the case pointer math
             * will work correctly on the resulting expression.
             */
            double diff = size_ety / size_cty;

            /*
             * This means the type cast type points to is bigger than the expression pointer type,
             * so we cannot allow this cast
             */
            if (diff < 1.0)
            {
                ERROR (
                    &exp->loc, 3015,
                    "Cannot cast '%s' to bigger pointer type '%s'",
                    GetQTypeName (ety, NULL)->data,
                    GetQTypeName (cty, NULL)->data);
            }
            /*
             * This means the underlying pointer types mismatch completely
             */
            else if ((((double) (uint64_t)diff) - diff) != 0.0)
            {
                ERROR (
                    &exp->loc, 3015,
                    "Cannot cast '%s' to unmatched pointer type '%s'",
                    GetQTypeName (cty, NULL)->data,
                    GetQTypeName (ety, NULL)->data);
            }
        }
        /*
         * Others casts are not permitted
         */
        else
        {
            ERROR (
                &exp->loc, 3015,
                "Cannot cast '%s' to '%s'",
                GetQTypeName (ety, NULL)->data,
                GetQTypeName (cty, NULL)->data);
        }
        // primitives?

        sexp.ty = cty;
        return sexp;
    }
    case A_asmExp:
    {
        SemantMIPS_Translate (context->module, exp->u.assembly.code);
        return Expression_New (Tr_Asm (exp->u.assembly.code), Ty_Void());
    }
    case A_asmExpOld:
    {
        const char * data = exp->u.asmOld.data;
        U_stringList dst = exp->u.asmOld.dst;
        U_stringList src = exp->u.asmOld.src;

        return Expression_New (
                   Tr_AsmOld (
                       exp->u.asmOld.code,
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
                    ERROR_UNEXPECTED_TYPE (&al->head->loc, t, sexp.ty);
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
        return Expression_New (
                   Tr_String (context, exp->u.stringg),
                   GetOrCreateTypeEntry (context, Ty_Pointer (Ty_Str())));
    }
    case A_opExp:
    {
        struct A_opExp_t opExp = exp->u.op;

        A_oper oper = opExp.oper;

        Semant_Exp left = TransExp (context, opExp.left);
        left.ty = GetActualType (left.ty);

        Semant_Exp right = TransExp (context, opExp.right);
        right.ty = GetActualType (right.ty);

        // do type checks
        if (!is_int (left) && !is_int (right))
        {
            /*
             * Only int type allow perform math and logic, so we change the type of the operands
             * to int, spawn an error and continue.
             */
            if (oper != A_eqOp && oper != A_neqOp)
            {
                ERROR_UNEXPECTED_TYPE (&opExp.left->loc, Ty_Int(), left.ty);
                ERROR_UNEXPECTED_TYPE (&opExp.right->loc, Ty_Int(), right.ty);
                left.ty = Ty_Int();
                right.ty = Ty_Int();
            }
            /*
             * Two pointers can be compared by their addresses
             */
            else if (left.ty->meta.is_pointer && right.ty->meta.is_pointer)
            {
                left.ty = Ty_Int();
                right.ty = Ty_Int();
            }
            /*
             * Pointer types can be compared with nil, and the actual comprising is based on the
             * nil value and value(address) of the nillable object handle
             */
            else if ((left.ty->meta.is_pointer && right.ty == Ty_Nil())
                     || (right.ty->meta.is_pointer && left.ty == Ty_Nil()))
            {
                left.ty = Ty_Int();
                right.ty = Ty_Int();
            }
            /*
             * In case of comprising of nil and non-nillable value we change the type of the bad
             * value to int, spawn error and parse further
             */
            else if (left.ty == Ty_Nil())
            {
                ERROR (
                    &opExp.right->loc,
                    3001,
                    "Expected pointer type got '%s'",
                    left.ty->meta.name);
                right.ty = Ty_Int();
            }
            else if (right.ty == Ty_Nil())
            {
                ERROR (
                    &opExp.left->loc,
                    3001,
                    "Expected pointer type got '%s'",
                    left.ty->meta.name);
                left.ty = Ty_Int();
            }
            else if (left.ty->meta.is_handle || left.ty->meta.is_handle)
            {
                ERROR_UNEXPECTED_TYPE (&opExp.left->loc, Ty_Int(), left.ty);
                ERROR_UNEXPECTED_TYPE (&opExp.right->loc, Ty_Int(), right.ty);
                left.ty = Ty_Int();
                right.ty = Ty_Int();
            }
            else
            {
                assert (0);
            }
        }
        /*
         * We do type recovery here to parse the unit further assuming these operations are
         * applicable for int only.
         */
        else if (!is_int (left) || !is_int (right))
        {
            // TODO constant expression if int was literal
            if (left.ty->meta.is_pointer || right.ty->meta.is_pointer)
            {
                /*
                 * Only addition and substitution is allowed for pointers, if something else was
                 * passed we correct it and proceed parsing.
                 */
                if (oper != A_plusOp && oper != A_minusOp)
                {
                    ERROR_INVALID_EXPRESSION (&exp->loc);
                    oper = A_plusOp;
                }

                if (left.ty->meta.is_pointer)
                {
                    right.exp = Tr_Op (A_timesOp,
                                       right.exp,
                                       Tr_Int (Ty_SizeOf (left.ty->u.pointer)),
                                       Ty_Int());
                }
                else
                {
                    left.exp = Tr_Op (A_timesOp,
                                      left.exp,
                                      Tr_Int (Ty_SizeOf (right.ty->u.pointer)),
                                      Ty_Int());
                }
            }
            else
            {
                if (!is_int (left))
                {
                    ERROR_UNEXPECTED_TYPE (&opExp.left->loc, Ty_Int(), left.ty);
                    left.ty = Ty_Int();
                }
                if (!is_int (right))
                {
                    ERROR_UNEXPECTED_TYPE (&opExp.right->loc, Ty_Int(), right.ty);
                    right.ty = Ty_Int();
                }
            }
        }

        return Expression_New (Tr_Op (oper, left.exp, right.exp, left.ty), left.ty);
    }
    case A_arrayExp:
    case A_recordExp:
    {
        return TransHandleExp (context, exp, NULL);
    }
    case A_assignExp:
    {
        struct A_assignExp_t assignExp = exp->u.assign;

        Semant_Exp lexp = TransVar (context, assignExp.var, TRUE);
        lexp.ty = GetActualType (lexp.ty);

        Semant_Exp rexp = TransExp (context, assignExp.exp);
        rexp.ty = GetActualType (rexp.ty);

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
        // Unmatched types require special handling
        else if (lexp.ty != rexp.ty)
        {
            // If lexp is pointer and rexp is nil then everything is fine
            if (! (rexp.ty == Ty_Nil() && lexp.ty->meta.is_pointer))
            {
                ERROR_UNEXPECTED_TYPE (&assignExp.exp->loc, lexp.ty, rexp.ty)
                rexp.ty = lexp.ty;
            }
        }

        Tr_exp aexp = NULL;

        /*
         * There are two possible assignment scenarios:
         *
         *  1. The variable is assigned a simple value, like int or pointer, in this case we
         *     do simple temp-to-temp move.
         *
         *  2. The variable is assigned another variable that was initialized with an array or
         *     record expression(basically any var that is handle in nature). In this case we
         *     need to allocate the same array on stack and do deep copy of the var.
         *
         *  In the first case we simply do temp-to-temp move, in the second case we do deep copy.
         */

        /*
         * If the rhs is a variable and the value it yields is handle we copy the whole array.
         */
        bool copy = (assignExp.exp->kind == A_varExp || assignExp.exp->kind == A_valueAt)
                    && rexp.ty->meta.is_handle;

        /*
         * The difference here is in the type size, if we copy the stack array we allocate full
         * size, if we just move handle we allocate word-size temp.
         */
        if (copy)
        {
            aexp = Tr_Memcpy (lexp.exp, rexp.exp, Ty_SizeOf (rexp.ty) / F_wordSize);
            aexp = Tr_Seq (aexp, lexp.exp); //assignment yields a value everytime for now
        }
        else
        {
            aexp = Tr_Assign (lexp.exp, rexp.exp);
            aexp = Tr_Seq (aexp, lexp.exp); //assignment yields a value everytime for now
        }

        return Expression_New (aexp, rexp.ty);
    }
    case A_ifExp:
    {
        struct A_ifExp_t ifExp = exp->u.iff;

        Semant_Exp texp = TransExp (context, ifExp.test);

        // currenlty Helium does not support bool
        if (texp.ty != Ty_Int())
        {
            ERROR_UNEXPECTED_TYPE (&ifExp.test->loc, Ty_Int(), texp.ty);
        }

        Semant_Exp pexp = e_void;
        if (ifExp.tr)
        {
            S_BeginScope (context->venv);

            pexp = TransScope (context, ifExp.tr);

            S_EndScope (context->venv);
        }

        Semant_Exp nexp = e_void;
        if (ifExp.fl)
        {
            S_BeginScope (context->venv);

            nexp = TransScope (context, ifExp.fl);

            S_EndScope (context->venv);
        }

        return Expression_New (Tr_If (texp.exp, pexp.exp, nexp.exp), nexp.ty);
    }
    case A_whileExp:
    {
        struct A_whileExp_t whileExp = exp->u.whilee;

        Semant_Exp texp = TransExp (context, whileExp.test);

        // currenlty Helium does not support bool
        if (texp.ty != Ty_Int())
        {
            ERROR_UNEXPECTED_TYPE (&whileExp.test->loc, Ty_Int(), texp.ty);
        }

        Temp_label outter_breaker = context->breaker;
        Temp_label breaker = Temp_NewLabel();
        context->breaker = breaker;
        context->loopNesting++;

        S_BeginScope (context->venv);

        Semant_Exp body = TransScope (context, whileExp.body);

        S_EndScope (context->venv);

        context->loopNesting--;
        context->breaker = outter_breaker;

        return Expression_New (Tr_While (texp.exp, body.exp, breaker), Ty_Void());
    }
    case A_forExp:
    {
        struct A_forExp_t forExp = exp->u.forr;

        Semant_Exp lexp = TransExp (context, forExp.lo);
        if (!is_int (lexp))
        {
            ERROR_UNEXPECTED_TYPE (&forExp.lo->loc, Ty_Int(), lexp.ty)
        }

        Semant_Exp hexp = TransExp (context, forExp.hi);
        if (!is_int (hexp))
        {
            ERROR_UNEXPECTED_TYPE (&forExp.hi->loc, Ty_Int(), hexp.ty)
        }

        S_BeginScope (context->venv);

        // store the iterator within the new scope
        Env_Entry entry = Env_VarEntryNew (
                              context->level,
                              Tr_Alloc (context->level, Ty_Int(), forExp.var, forExp.escape),
                              Ty_Int());
        S_Enter (context->venv, forExp.var, entry);

        Temp_label label = context->breaker;
        Temp_label breaker = Temp_NewLabel();
        context->breaker = breaker;
        context->loopNesting++;

        Semant_Exp body = TransScope (context, forExp.body);

        context->loopNesting--;
        context->breaker = label;

        S_EndScope (context->venv);

        return Expression_New (Tr_For (
                                   lexp.exp,
                                   hexp.exp,
                                   body.exp,
                                   entry->u.var.access,
                                   breaker),
                               Ty_Void());
    }
    case A_breakExp:
    {
        assert (context->loopNesting >= 0);

        if (!context->loopNesting)
        {
            ERROR (&exp->loc, 3010, "Unexpected break", "");
            return e_void;
        }
        else
        {
            return Expression_New (Tr_Break (context->breaker), Ty_Void());
        }
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

    Escape_Find (m->ast);

    struct Semant_ContextType context;
    context.module = m;
    context.loopNesting = 0;

    Env_Init (&context);
    Tr_Init (&context);

    LIST_FOREACH (dec, m->ast) TransDec (&context, dec);

    return Vector_Size (&m->errors.semant);
}
