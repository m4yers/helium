#include <string.h>
#include <assert.h>

#include "ext/list.h"
#include "ext/str.h"
#include "ext/mem.h"

#include "semant.h"
#include "translate.h"
#include "mipsmachine.h"
#include "frame.h"
#include "tree.h"

typedef struct patchList_ * patchList;
struct patchList_
{
    Temp_label * head;
    patchList tail;
};

Tr_expList Tr_ExpList (Tr_exp head, Tr_expList tail)
{
    Tr_expList r = checked_malloc (sizeof (*r));
    r->head = head;
    r->tail = tail;
    return r;
}

static patchList PatchList (Temp_label * head, patchList tail)
{
    patchList r = checked_malloc (sizeof (*r));
    r->head = head;
    r->tail = tail;
    return r;
}

static void Tr_Patch (patchList list, Temp_label label)
{
    // TODO count the number of patches
    for (; list; list = list->tail)
    {
        * (list->head) = label;
    }
}

struct Cx
{
    patchList trues;
    patchList falses;
    T_stm stm;
};

struct Tr_exp_
{
    enum { Tr_ex, Tr_sx, Tr_cx } kind;
    union
    {
        T_exp ex;
        T_stm sx;
        struct Cx cx;
    } u;
};

static Tr_exp Tr_Ex (T_exp exp)
{
    Tr_exp r = checked_malloc (sizeof (*r));
    r->kind = Tr_ex;
    r->u.ex = exp;
    return r;
}

static Tr_exp Tr_Sx (T_stm stm)
{
    Tr_exp r = checked_malloc (sizeof (*r));
    r->kind = Tr_sx;
    r->u.sx = stm;
    return r;
}

static Tr_exp Tr_Cx (patchList trues, patchList falses, T_stm stm)
{
    Tr_exp r = checked_malloc (sizeof (*r));
    r->kind = Tr_cx;
    r->u.cx.trues = trues;
    r->u.cx.falses = falses;
    r->u.cx.stm = stm;
    return r;
}

static T_exp Tr_UnEx (Tr_exp exp)
{
    assert (exp);
    switch (exp->kind)
    {
    case Tr_ex:
    {
        return exp->u.ex;
    }
    case Tr_sx:
    {
        /* printf("Tr_UnEx -> Tr_sx"); */
        /* assert (0); */
        /* break; */
        return T_Eseq (exp->u.sx, Tr_UnEx (Tr_Void()));
    }
    case Tr_cx:
    {
        Temp_temp r = Temp_NewTemp();
        Temp_label t = Temp_NewLabel(), f = Temp_NewLabel();

        /*
         * we patch the logical statments so we can control its true and false
         * branches from this translation
         */
        // TODO if we have unused label the ESEQ below MUST be much simpler
        Tr_Patch (exp->u.cx.trues, t);
        Tr_Patch (exp->u.cx.falses, f);

        // HMM... it is possible use less instr for GT and LT for MIPS using slt and slti

        // initialize the result with one which makes it TRUE
        return T_Eseq (T_Move (T_Temp (r), T_Const (1)),

                       // execute the logical statements
                       T_Eseq (exp->u.cx.stm,

                               // if the stm decides jump to FALSE it will arrive here
                               T_Eseq (T_Label (f),

                                       // here we clear the initial value so the actual result is FALSE
                                       T_Eseq (T_Move (T_Temp (r), T_Const (0)),

                                               // if the stm decides jump to TRUE we do not clear the result and it stays TRUE
                                               T_Eseq (T_Label (t),

                                                       // the result fo this logical evaluation returned here as an expression
                                                       T_Temp (r))))));
    }
    }
}

static T_stm Tr_UnSx (Tr_exp exp)
{
    assert (exp);
    switch (exp->kind)
    {
    case Tr_ex:
    {
        T_exp ex = exp->u.ex;
        switch (ex->kind)
        {
        // is it really needed?
        /* case T_ESEQ: */
        /* { */
        /*     return T_Seq (ex->u.ESEQ.stm, T_Exp (ex->u.ESEQ.exp)); */
        /* } */
        default:
        {
            return T_Exp (ex);
        }

        }
    }
    case Tr_sx:
    {
        return exp->u.sx;
    }
    case Tr_cx:
    {
        printf ("Tr_UnSx -> Tr_cx");
        assert (0);
    }
    }
}

static struct Cx Tr_UnCx (Tr_exp exp)
{
    assert (exp);
    switch (exp->kind)
    {
    case Tr_ex:
    {
        printf ("Tr_UnCx -> Tr_ex");
        assert (0);
    }
    case Tr_sx:
    {
        printf ("Tr_UnCx -> Tr_sx");
        assert (0);
    }
    case Tr_cx:
    {
        return exp->u.cx;
    }
    }
}

struct Tr_access_
{
    Tr_level level;
    F_access access;
};

static Tr_access Tr_Access (Tr_level level, F_access access)
{
    Tr_access r = checked_malloc (sizeof (*r));
    r->level = level;
    r->access = access;
    return r;
}

static Tr_accessList Tr_AccessList (Tr_access head, Tr_accessList tail)
{
    Tr_accessList r = checked_malloc (sizeof (*r));
    r->head = head;
    r->tail = tail;
    return r;
}

Tr_level Tr_NewLevel (Tr_level parent, Temp_label name, U_boolList formals)
{
    Tr_level r = checked_malloc (sizeof (*r));
    r->name = name;
    r->parent = parent;
    // adding static link in the beginning
    r->frame = F_NewFrame (name, U_BoolList (TRUE, formals));

    r->formals = NULL;
    for (F_accessList l = F_Formals (r->frame); l; l = l->tail)
    {
        Tr_access access = Tr_Access (r, l->head);

        LIST_PUSH (r->formals, access);
    }

    return r;
}

Temp_label Tr_ScopedLabel (Tr_level level, const char * name)
{
    if (strcmp (name, "main") == 0)
    {
        return Temp_NamedLabel (name);
    }

    char * s = checked_malloc (20 + strlen (level->name->name) + strlen (name));
    sprintf (s, "__%s__%s", level->name->name, name);

    return Temp_NamedLabel (s);
}

Tr_accessList Tr_Formals (Tr_level level)
{
    return level->formals->tail;
}

Tr_access Tr_AllocVirtual (Tr_level level, S_symbol name)
{
    Tr_access ta = Tr_Access (level, F_AllocVirtual (level->frame, name ? name->name : NULL));
    LIST_PUSH (level->locals, ta);

    return ta;
}

Tr_access Tr_AllocMaterialize (Tr_access access, Tr_level level, Ty_ty type, bool escape)
{
    int size = Ty_SizeOf (type);
    assert (size > 0);

    F_access fa;
    // FIXME currently assuming it evenly divides
    fa = F_AllocMaterializeArray (level->frame, access->access, size / F_wordSize, escape);

    return access;
}

Tr_access Tr_Alloc (Tr_level level, Ty_ty type, S_symbol name, bool escape)
{
    int size = Ty_SizeOf (type);
    assert (size > 0);

    F_access fa;
    if (size > F_wordSize)
    {
        // FIXME currently assuming it evenly divides
        // FIXME Frame module should not care about escaping, rename to onStack
        fa = F_AllocArray (level->frame, size / F_wordSize, name ? name->name : NULL, escape);
    }
    else
    {
        fa = F_Alloc (level->frame, name ? name->name : NULL, escape);
    }
    Tr_access ta = Tr_Access (level, fa);
    LIST_PUSH (level->locals, ta);

    return ta;
}

/***************
 *  Variables  *
 ***************/

Tr_exp Tr_SimpleVar (Tr_access access, Tr_level level, bool deref)
{
    T_exp framePtr = T_Temp (F_FP());
    Tr_level current = level;
    while (current != access->level)
    {
        assert (current);
        framePtr = T_Mem (framePtr);
        current = current->parent;
    }
    return Tr_Ex (F_GetVar (access->access, framePtr, deref));
}

// FIXME codegen MUST detedct this pattern: add, add .. add, mem
Tr_exp Tr_FieldVar (Tr_exp var, Ty_ty type, S_symbol field, bool deref)
{
    int offset = 0;
    LIST_FOREACH (f, type->u.record)
    {
        if (f->name == field)
        {
            break;
        }

        offset += Ty_SizeOf (f->ty);
    }

    T_exp exp = T_Binop (
                    T_plus,
                    Tr_UnEx (var),
                    T_Const (offset));
    if (deref)
    {
        exp = T_Mem (exp);
    }

    return Tr_Ex (exp);
}

Tr_exp Tr_DerefExp (Tr_exp exp)
{
    return Tr_Ex (T_Mem (Tr_UnEx (exp)));
}

Tr_exp Tr_SubscriptVar (Tr_exp var, Ty_ty type, Tr_exp subscript, bool deref)
{
    // TODO: what about range checks?
    T_exp exp = T_Binop (
                    T_plus,
                    Tr_UnEx (var),
                    T_Binop (
                        T_mul,
                        Tr_UnEx (subscript),
                        T_Const (Ty_SizeOf (type->u.array.type))));

    if (deref)
    {
        exp = T_Mem (exp);
    }

    return Tr_Ex (exp);
}

/*****************
 *  Expressions  *
 *****************/

Tr_exp Tr_Seq (Tr_exp seq, Tr_exp current)
{
    if (seq)
    {
        return Tr_Ex (T_Eseq (Tr_UnSx (seq), Tr_UnEx (current)));
    }
    else
    {
        return current;
    }
}

Tr_exp Tr_Call (Temp_label label, Tr_level encolosing, Tr_level own, Tr_expList args)
{
    T_expList el = NULL;
    LIST_FOREACH (arg, args)
    {
        LIST_PUSH (el, Tr_UnEx (arg))
    }

    // pass data to frame handler to do some magic there
    F_ProcFunctionCall (own->frame, encolosing->frame, el);

    // calculating static link offset
    T_exp link = T_Temp (F_FP());
    while (encolosing != own->parent)
    {
        assert (encolosing);
        link = T_Mem (link);
        encolosing = encolosing->parent;
    }

    return Tr_Ex (T_Call (T_Name (label), T_ExpList (link, el)));
}

Tr_exp Tr_Nil()
{
    return Tr_Ex (T_Nil());
}

Tr_exp Tr_Void()
{
    return Tr_Ex (T_Void());
}

Tr_exp Tr_Int (int value)
{
    return Tr_Ex (T_Const (value));
}

Tr_exp Tr_String (Semant_Context c, const char * value)
{
    Temp_label label = Temp_NewLabel();
    Program_AddFragment (c->module, F_StringFrag (label, value));
    return Tr_Ex (T_Name (label));
}

Tr_exp Tr_Op (A_oper op, Tr_exp left, Tr_exp right, Ty_ty ty)
{
    (void) ty;
    switch (op)
    {
    case A_plusOp:
    {
        return Tr_Ex (T_Binop (T_plus, Tr_UnEx (left), Tr_UnEx (right)));
    }
    case A_minusOp:
    {
        return Tr_Ex (T_Binop (T_minus, Tr_UnEx (left), Tr_UnEx (right)));
    }
    case A_timesOp:
    {
        return Tr_Ex (T_Binop (T_mul, Tr_UnEx (left), Tr_UnEx (right)));
    }
    case A_divideOp:
    {
        return Tr_Ex (T_Binop (T_div, Tr_UnEx (left), Tr_UnEx (right)));
    }
    case A_gtOp:
    {
        T_stm stm = T_Cjump (T_gt, Tr_UnEx (left), Tr_UnEx (right), NULL, NULL);
        patchList trues = PatchList (&stm->u.CJUMP.true, NULL);
        patchList falses = PatchList (&stm->u.CJUMP.false, NULL);
        return Tr_Cx (trues, falses, stm);
    }
    case A_ltOp:
    {
        T_stm stm = T_Cjump (T_lt, Tr_UnEx (left), Tr_UnEx (right), NULL, NULL);
        patchList trues = PatchList (&stm->u.CJUMP.true, NULL);
        patchList falses = PatchList (&stm->u.CJUMP.false, NULL);
        return Tr_Cx (trues, falses, stm);
    }
    case A_leOp:
    {
        T_stm stm = T_Cjump (T_le, Tr_UnEx (left), Tr_UnEx (right), NULL, NULL);
        patchList trues = PatchList (&stm->u.CJUMP.true, NULL);
        patchList falses = PatchList (&stm->u.CJUMP.false, NULL);
        return Tr_Cx (trues, falses, stm);
    }
    case A_geOp:
    {
        T_stm stm = T_Cjump (T_ge, Tr_UnEx (left), Tr_UnEx (right), NULL, NULL);
        patchList trues = PatchList (&stm->u.CJUMP.true, NULL);
        patchList falses = PatchList (&stm->u.CJUMP.false, NULL);
        return Tr_Cx (trues, falses, stm);
    }
    case A_eqOp:
    {
        T_stm stm = T_Cjump (T_eq, Tr_UnEx (left), Tr_UnEx (right), NULL, NULL);
        patchList trues = PatchList (&stm->u.CJUMP.true, NULL);
        patchList falses = PatchList (&stm->u.CJUMP.false, NULL);
        return Tr_Cx (trues, falses, stm);
    }
    case A_neqOp:
    {
        T_stm stm = T_Cjump (T_ne, Tr_UnEx (left), Tr_UnEx (right), NULL, NULL);
        patchList trues = PatchList (&stm->u.CJUMP.true, NULL);
        patchList falses = PatchList (&stm->u.CJUMP.false, NULL);
        return Tr_Cx (trues, falses, stm);
    }
    }
}

Tr_exp Tr_ArrayExp (Tr_access access, Ty_ty type, Tr_expList list, int offset)
{
    T_exp var = F_GetVar (access->access, T_Temp (F_FP()), TRUE);
    int ts = Ty_SizeOf (type->u.array.type);

    T_exp exp = T_Eseq (T_NoOp(), NULL);
    T_exp tail = exp;

    int i = 0;
    LIST_FOREACH (e, list)
    {
        T_exp init = Tr_UnEx (e);

        switch (type->u.array.type->kind)
        {
        case Ty_int:
        {
            tail = tail->u.ESEQ.exp =
                       T_Eseq (T_Move (T_Mem (
                                           T_Binop (
                                               T_plus,
                                               var,
                                               T_Const (offset + ts * i++))),
                                       init), NULL);
        }
        default:
        {
            tail = tail->u.ESEQ.exp = T_Eseq (T_Exp (init), NULL);
        }
        }
    }

    tail->u.ESEQ.exp = var;

    return Tr_Ex (exp);
}

Tr_exp Tr_RecordExp (Tr_access access, Ty_ty type, Tr_expList list, int offset)
{
    T_exp base = F_GetVar (access->access, T_Temp (F_FP()), TRUE);

    T_exp exp = T_Eseq (T_NoOp(), NULL);
    T_exp tail = exp;

    LIST_FOREACH (f, type->u.record)
    {
        T_exp init = Tr_UnEx (list->head);
        Ty_ty ft = GetActualType (f->ty);

        /*
         * nillible types can be initialized in two ways, either by copying existing memory block,
         * or by init expression that fills current mem block.
         */
        if (ft->meta.is_handle)
        {
            switch (init->kind)
            {
            case T_TEMP:
            {
                Tr_exp copy = Tr_Memcpy (
                                  Tr_Ex (T_Binop (
                                             T_plus,
                                             base,
                                             T_Const (offset))),
                                  list->head,
                                  Ty_SizeOf (ft) / F_wordSize);
                tail = tail->u.ESEQ.exp = T_Eseq (Tr_UnSx (copy), NULL);
                break;
            }
            case T_ESEQ:
            {
                tail = tail->u.ESEQ.exp = T_Eseq (T_Exp (init), NULL);
                break;
            }
            default:
            {
                assert (0);
            }
            }
        }
        /*
         * non-nillible types are just copied.
         */
        else
        {
            tail = tail->u.ESEQ.exp =
                       T_Eseq (T_Move (T_Mem (
                                           T_Binop (
                                               T_plus,
                                               base,
                                               T_Const (offset))),
                                       init), NULL);
        }

        offset += Ty_SizeOf (f->ty);
        list = list->tail;
    }

    tail->u.ESEQ.exp = base;

    return Tr_Ex (exp);
}

Tr_exp Tr_Assign (Tr_exp left, Tr_exp right)
{
    T_exp l = Tr_UnEx (left);
    T_exp r = Tr_UnEx (right);
    return Tr_Sx (T_Move (l, r));
}

Tr_exp Tr_Memcpy (Tr_exp dst, Tr_exp src, size_t words)
{
    T_exp d = Tr_UnEx (dst);
    T_exp s = Tr_UnEx (src);
    T_stm r = T_NoOp();

    while (words--)
    {
        r = T_Seq (T_Move (
                       T_Mem (
                           T_Binop (
                               T_plus,
                               d,
                               T_Const (F_wordSize * words))),
                       T_Mem (
                           T_Binop (
                               T_plus,
                               s,
                               T_Const (F_wordSize * words)))),
                   r);
    }

    return Tr_Sx (r);
}

Tr_exp Tr_If (Tr_exp test, Tr_exp te, Tr_exp fe)
{
    assert (test);
    assert (te);

    Temp_label t = Temp_NewLabel();
    Temp_label f = Temp_NewLabel();
    Temp_label e = Temp_NewLabel();
    Temp_temp r = Temp_NewTemp();

    if (fe)
    {
        /*
         * first we calculate the test, then check whether it is a zero,
         * if so, execute the 'false' branch, if something else, execute
         * the 'true' branch. In the end we return the result in 'r' register
         */
        return Tr_Ex (T_Eseq (T_Cjump (T_eq, Tr_UnEx (test), T_Const (0), f, t),
                              T_Eseq (T_Label (t),
                                      T_Eseq (T_Move (T_Temp (r), Tr_UnEx (te)),
                                              T_Eseq (T_Jump (T_Name (e), Temp_LabelList (e, NULL)),
                                                      T_Eseq (T_Label (f),
                                                              T_Eseq (T_Move (T_Temp (r), Tr_UnEx (fe)),
                                                                      T_Eseq (T_Label (e), T_Temp (r)))))))));
    }
    else
    {
        /*
         * same as above except if the test evaluates to zero we jump to the end.
         * The result is a statement instead of expression, so it cannot be
         * an r-value; though TODO: RTFM
         */
        return Tr_Sx (
                   // campare whatever Tr_UnEx(test) temp containts with zero
                   T_Seq (T_Cjump (T_eq, Tr_UnEx (test), T_Const (0), e, f),

                          // we always campare to zero so the FALSE is actually TRUE
                          T_Seq (T_Label (f),

                                 // if not zero execute the TRUE branch
                                 T_Seq (Tr_UnSx (te),

                                        // if equals to zero go to the end
                                        T_Label (e)))));
    }
}

Tr_exp Tr_While (Tr_exp test, Tr_exp body, Temp_label done)
{
    Temp_label t = Temp_NewLabel();
    Temp_label b = Temp_NewLabel();

    if (!done)
    {
        done = Temp_NewLabel();
    }

    /*
     * it is a while loop so we evaluate the test expression each time we jump
     * to the 't' label. After evaluation we compare the result to 0, if it is
     * equal to 0 we jump to the end, and execute the body otherwise with new
     * jump to the beginning of the snippet. The body expression can contain
     * additional jumps to the end of the snippet that was generated from break
     * statements.
     */
    return Tr_Sx (T_Seq (T_Label (t),
                         T_Seq (T_Cjump (T_eq, Tr_UnEx (test), T_Const (0), done, b),
                                T_Seq (T_Label (b),
                                       T_Seq (Tr_UnSx (body),
                                               T_Seq (T_Jump (T_Name (t), Temp_LabelList (t, NULL)),
                                                       T_Label (done)))))));
}

Tr_exp Tr_For (Tr_exp lo, Tr_exp hi, Tr_exp body, Tr_access iter, Temp_label done)
{
    T_exp l = F_GetVar (iter->access, T_Temp (F_FP()), TRUE);
    Temp_temp h = Temp_NewTemp();
    Temp_label c = Temp_NewLabel();
    Temp_label t = Temp_NewLabel();
    Temp_label n = Temp_NewLabel();

    if (!done)
    {
        done = Temp_NewLabel();
    }

    /*
     * first we calculate boundries and store them to 'l' and 'h' registers,
     * then check whether we can enter the loop. After execution of the body
     * expression we need to check again wheter we reached the upper limit,
     * thus we escape possible overflow increment of the left bound.
     */
    return Tr_Sx (T_Seq (T_Move (l, Tr_UnEx (lo)),
                         T_Seq (T_Move (T_Temp (h), Tr_UnEx (hi)),
                                T_Seq (T_Label (c),
                                       T_Seq (T_Cjump (T_le, l, T_Temp (h), t, done),
                                               T_Seq (T_Label (t),
                                                       T_Seq (Tr_UnSx (body),
                                                               T_Seq (T_Cjump (T_eq, l, T_Temp (h), done, n),
                                                                       T_Seq (T_Label (n),
                                                                               T_Seq (T_Move (l, T_Binop (T_plus, l, T_Const (1))),
                                                                                       T_Seq (T_Jump (T_Name (c), Temp_LabelList (c, NULL)),
                                                                                               T_Label (done))))))))))));
}

Tr_exp Tr_Break (Temp_label done)
{
    return Tr_Sx (T_Jump (T_Name (done), Temp_LabelList (done, NULL)));
}

Tr_exp Tr_Ret (Tr_level level, Tr_exp exp)
{
    struct String_t name = String (F_Name (level->frame)->name);
    if (String_Equal (&name, "main"))
    {
        return Tr_Sx (T_Exit (Tr_UnEx (exp)));
    }
    else
    {
        Temp_label ret = F_Ret (level->frame);
        return Tr_Sx (T_Seq (
                          T_Move (T_Temp (F_RV()), Tr_UnEx (exp)),
                          T_Jump (T_Name (ret), Temp_LabelList (ret, NULL))));
    }
}

Tr_exp Tr_Exit (Tr_exp exp)
{
    return Tr_Sx (T_Exit (Tr_UnEx (exp)));
}

Tr_exp Tr_Asm (const char * code, Tr_exp data, U_stringList dst, U_stringList src)
{
    // HMM... is it the right place?
    Temp_tempList dl = NULL;
    LIST_FOREACH (d, dst)
    {
        Temp_temp t = F_RegistersGet_s (regs_all, d);
        if (!t)
        {
            assert (0);
        }

        LIST_PUSH (dl, t);
    }

    Temp_tempList sl = NULL;
    LIST_FOREACH (d, src)
    {
        Temp_temp t = F_RegistersGet_s (regs_all, d);
        if (!t)
        {
            assert (0);
        }

        LIST_PUSH (sl, t);
    }

    return Tr_Sx (T_Asm (code, data ? Tr_UnEx (data) : NULL, dl, sl));
}

void Tr_Init (Semant_Context c)
{
    c->global = c->level = Tr_NewLevel (NULL, Temp_NamedLabel ("global"), NULL);
}

void Tr_ProcEntryExit (Semant_Context c, Tr_level level, Tr_exp body)
{
    T_stm stm = NULL;

    stm = F_ProcEntryExit1 (level->frame, Tr_UnSx (body));

    Program_AddFragment (c->module, F_ProcFrag (stm, level->frame));
}
