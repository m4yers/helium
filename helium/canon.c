#include <stdio.h>
#include <assert.h>

#include "ext/util.h"
#include "ext/mem.h"
#include "ext/bool.h"

#include "symbol.h"
#include "temp.h"
#include "tree.h"
#include "canon.h"

typedef struct expRefList_ * expRefList;
struct expRefList_
{
    T_exp * head;
    expRefList tail;
};

/* local function prototypes */
static T_stm DoStm (T_stm stm);
static struct stmExp DoExp (T_exp exp);
static C_stmListList MakeBlocks (T_stmList stms, Temp_label done);
static T_stmList GetNext (void);

static expRefList ExpRefList (T_exp * head, expRefList tail)
{
    expRefList p = (expRefList) checked_malloc (sizeof * p);
    p->head = head;
    p->tail = tail;
    return p;
}

static bool isNop (T_stm x)
{
    return x->kind == T_EXP && x->u.EXP->kind == T_CONST;
}

static T_stm Seq (T_stm x, T_stm y)
{
    if (isNop (x))
    {
        return y;
    }
    if (isNop (y))
    {
        return x;
    }
    return T_Seq (x, y);
}

static bool Commute (T_exp y, T_stm x)
{
    if (isNop (x))
    {
        return TRUE;
    }
    if (y->kind == T_NAME || y->kind == T_CONST)
    {
        return TRUE;
    }
    return FALSE;
}

struct stmExp
{
    T_stm s;
    T_exp e;
};

/*
 * The function bubbles up the statements from the expression reference list and returns them as
 * a new sequance of statements.
 */
static T_stm Reorder (expRefList rlist)
{
    if (!rlist)
    {
        /* noOp */
        return T_NoOp();
    }
    /*
     * If the head is a function call we store the result in a temp, converting the call into
     * the statement, and creating ESEQ(s, e) pair in hope that the successive call to Reorder
     * will clean this up.
     *
     * HMM: though I don't really get why, can't we just return seq(s, noOp)?
     */
    else if ((*rlist->head)->kind == T_CALL)
    {
        Temp_temp t = Temp_NewTemp();
        *rlist->head = T_Eseq (T_Move (T_Temp (t), *rlist->head), T_Temp (t));
        return Reorder (rlist);
    }
    else
    {
        struct stmExp hd = DoExp (*rlist->head);
        T_stm s = Reorder (rlist->tail);

        /*
         * This asks whether 's' uses anything from 'hd.e' computation, if it does not we can
         * safely put 's' into the result sequance which affectively makes it precede 'hd.e'
         */
        if (Commute (hd.e, s))
        {
            *rlist->head = hd.e;
            return Seq (hd.s, s);
        }
        /*
         * Otherwise we need calculate and save the result of 'hd.e' into a temp before 's',
         * which converts 'hd.e' into a statement; the 'hd.e' placeholder will acquire a new
         * pointer to a T_Temp expression returning result of 'hd.e' calculations.
         */
        else
        {
            Temp_temp t = Temp_NewTemp();
            *rlist->head = T_Temp (t);
            return Seq (hd.s, Seq (T_Move (T_Temp (t), hd.e), s));
        }
    }
}

static expRefList GetCallRList (T_exp exp)
{
    expRefList rlist, curr;
    T_expList args = exp->u.CALL.args;
    curr = rlist = ExpRefList (&exp->u.CALL.fun, NULL);
    for (; args; args = args->tail)
    {
        curr = curr->tail = ExpRefList (&args->head, NULL);
    }
    return rlist;
}

static struct stmExp StmExp (T_stm stm, T_exp exp)
{
    struct stmExp x;
    x.s = stm;
    x.e = exp;
    return x;
}

static struct stmExp DoExp (T_exp exp)
{
    switch (exp->kind)
    {
    case T_BINOP:
    {
        return StmExp (Reorder (
                           ExpRefList (&exp->u.BINOP.left,
                                       ExpRefList (&exp->u.BINOP.right, NULL))), exp);
    }
    case T_MEM:
    {
        return StmExp (Reorder (ExpRefList (&exp->u.MEM, NULL)), exp);
    }
    case T_ESEQ:
    {
        struct stmExp x = DoExp (exp->u.ESEQ.exp);
        return StmExp (Seq (DoStm (exp->u.ESEQ.stm), x.s), x.e);
    }
    case T_CALL:
    {
        return StmExp (Reorder (GetCallRList (exp)), exp);
    }
    default:
    {
        return StmExp (Reorder (NULL), exp);
    }
    }
}

static T_stm DoStm (T_stm stm)
{
    switch (stm->kind)
    {
    case T_SEQ:
    {
        return Seq (DoStm (stm->u.SEQ.left), DoStm (stm->u.SEQ.right));
    }
    case T_JUMP:
    {
        return Seq (Reorder (ExpRefList (&stm->u.JUMP.exp, NULL)), stm);
    }
    case T_RJUMP:
    {
        // HMM currently it has T_Temp node
        return stm;
    }
    case T_CJUMP:
    {
        return Seq (Reorder (
                        ExpRefList (&stm->u.CJUMP.left,
                                    ExpRefList (&stm->u.CJUMP.right, NULL))), stm);
    }
    case T_MOVE:
    {
        if (stm->u.MOVE.dst->kind == T_TEMP && stm->u.MOVE.src->kind == T_CALL)
        {
            return Seq (Reorder (GetCallRList (stm->u.MOVE.src)), stm);
        }
        else if (stm->u.MOVE.dst->kind == T_TEMP)
        {
            return Seq (Reorder (ExpRefList (&stm->u.MOVE.src, NULL)), stm);
        }
        else if (stm->u.MOVE.dst->kind == T_MEM)
        {
            return Seq (Reorder (
                            ExpRefList (&stm->u.MOVE.dst->u.MEM,
                                        ExpRefList (&stm->u.MOVE.src, NULL))), stm);
        }
        /*
         * If 'dst' is an ESEQ, for some reason, the stm part is returned as part of the result
         * sequance, the exp part becomes 'dst'
         */
        else if (stm->u.MOVE.dst->kind == T_ESEQ)
        {
            T_stm s = stm->u.MOVE.dst->u.ESEQ.stm;
            stm->u.MOVE.dst = stm->u.MOVE.dst->u.ESEQ.exp;
            return DoStm (T_Seq (s, stm));
        }

        /*
         * 'dst' SHOULD not be other than T_Temp or T_Mem
         */
        assert (0);
    }
    case T_EXP:
    case T_EXIT:
    {
        if (stm->u.EXP->kind == T_CALL)
        {
            return Seq (Reorder (GetCallRList (stm->u.EXP)), stm);
        }
        else
        {
            return Seq (Reorder (ExpRefList (&stm->u.EXP, NULL)), stm);
        }
    }
    default:
    {
        return stm;
    }
    }
}

/*
 * The function repeatedly converts a STM tree into a STM list. If the 'stm' is a T_SEQ node it
 * is being split an called Linear on the parts repeatedly. The end result is accumulated in the
 * T_stmList and passed as the second argument to each recursive call.
 */
static T_stmList Linear (T_stm stm, T_stmList right)
{
    if (stm->kind == T_SEQ)
    {
        return Linear (stm->u.SEQ.left, Linear (stm->u.SEQ.right, right));
    }
    else
    {
        return T_StmList (stm, right);
    }
}

T_stmList C_Linearize (T_stm stm)
{
    return Linear (DoStm (stm), NULL);
}

static C_stmListList StmListList (T_stmList head, C_stmListList tail)
{
    C_stmListList p = (C_stmListList) checked_malloc (sizeof * p);
    p->head = head;
    p->tail = tail;
    return p;
}

static C_stmListList Next (T_stmList list, T_stmList tail, Temp_label done)
{
    if (!tail)
    {
        return Next (
                   list,
                   T_StmList (T_Jump (T_Name (done), Temp_LabelList (done, NULL)), NULL),
                   done);
    }

    if (tail->head->kind == T_JUMP || tail->head->kind == T_CJUMP)
    {
        C_stmListList block;
        list->tail = tail;
        block = MakeBlocks (tail->tail, done);
        tail->tail = NULL;
        return block;
    }
    else if (tail->head->kind == T_LABEL)
    {
        Temp_label lab = tail->head->u.LABEL;
        return Next (
                   list,
                   T_StmList (T_Jump (T_Name (lab), Temp_LabelList (lab, NULL)), tail),
                   done);
    }
    else
    {
        list->tail = tail;
        return Next (tail, tail->tail, done);
    }
}

static C_stmListList MakeBlocks (T_stmList list, Temp_label done)
{
    if (!list)
    {
        return NULL;
    }

    /*
     * If there is no label at the beginnign - create one.
     */
    if (list->head->kind != T_LABEL)
    {
        return MakeBlocks (T_StmList (T_Label (Temp_NewLabel()), list), done);
    }

    return StmListList (list, Next (list, list->tail, done));
}

struct C_block C_BasicBlocks (T_stmList stmList)
{
    struct C_block b;
    b.label = Temp_NewLabel();
    b.stmLists = MakeBlocks (stmList, b.label);

    return b;
}

static S_table block_env;
static struct C_block global_block;

/**
 *
 * In fact the function returns next to last node
 */
static T_stmList GetLast (T_stmList list)
{
    T_stmList last = list;
    while (last->tail->tail)
    {
        last = last->tail;
    }
    return last;
}

static void Trace (T_stmList list)
{
    T_stmList last = GetLast (list);
    T_stm lab = list->head;
    T_stm s = last->tail->head;
    S_Enter (block_env, lab->u.LABEL, NULL);
    if (s->kind == T_JUMP)
    {
        T_stmList target = (T_stmList)S_Look (block_env, s->u.JUMP.jumps->head);
        if (!s->u.JUMP.jumps->tail && target)
        {
            last->tail = target; /* merge the 2 lists removing JUMP stm */
            Trace (target);
        }
        else
        {
            last->tail->tail = GetNext();    /* merge and keep JUMP stm */
        }
    }
    /* we want false label to follow CJUMP */
    else if (s->kind == T_CJUMP)
    {
        T_stmList true = (T_stmList)S_Look (block_env, s->u.CJUMP.true);
        T_stmList false = (T_stmList)S_Look (block_env, s->u.CJUMP.false);
        if (false)
        {
            last->tail->tail = false;
            Trace (false);
        }
        else if (true)   /* convert so that existing label is a false label */
        {
            last->tail->head = T_Cjump (
                                   T_notRel (s->u.CJUMP.op),
                                   s->u.CJUMP.left,
                                   s->u.CJUMP.right,
                                   s->u.CJUMP.false,
                                   s->u.CJUMP.true);

            last->tail->tail = true;
            Trace (true);
        }
        else
        {
            Temp_label f = Temp_NewLabel();

            last->tail->head = T_Cjump (
                                   s->u.CJUMP.op,
                                   s->u.CJUMP.left,
                                   s->u.CJUMP.right,
                                   s->u.CJUMP.true,
                                   f);

            last->tail->tail = T_StmList (T_Label (f), GetNext());
        }
    }
    else
    {
        assert (0);
    }
}

/*
 * get the Next block from the list of stmLists, using only those that have not been traced yet
 */
static T_stmList GetNext()
{
    if (!global_block.stmLists)
    {
        // FIXME this creates unneeded label
        return T_StmList (T_Label (global_block.label), NULL);
    }
    else
    {
        T_stmList s = global_block.stmLists->head;
        if (S_Look (block_env, s->head->u.LABEL)) /* label exists in the table */
        {
            Trace (s);
            return s;
        }
        else
        {
            global_block.stmLists = global_block.stmLists->tail;
            return GetNext();
        }
    }
}

T_stmList C_TraceSchedule (struct C_block b)
{
    C_stmListList sList;
    block_env = S_Empty();
    global_block = b;

    for (sList = global_block.stmLists; sList; sList = sList->tail)
    {
        S_Enter (block_env, sList->head->head->u.LABEL, sList->head);
    }

    return GetNext();
}
