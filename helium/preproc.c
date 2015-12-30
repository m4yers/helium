#include <assert.h>

#include "ext/str.h"

#include "ast.h"
#include "preproc.h"
#include "temp.h"

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

#define ASM(code,dst,src,data) { LIST_PUSH (l, A_AsmExpOld (&exp->loc,code,dst,src,data)); }
#define SL(head,tail) U_StringList(head,tail)

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
            LIST_PUSH (l, TransExp (context, e));
        }

        exp->u.seq = l;
        return exp;
    }
    case A_ifExp:
    {
        if (exp->u.iff.test)
        {
            TransExp (context, exp->u.iff.test);
        }

        if (exp->u.iff.tr)
        {
            TransScope (context, exp->u.iff.tr);
        }

        if (exp->u.iff.fl)
        {
            TransScope (context, exp->u.iff.fl);
        }

        return exp;
    }
    case A_whileExp:
    {
        if (exp->u.whilee.test)
        {
            TransExp (context, exp->u.whilee.test);
        }

        if (exp->u.whilee.body)
        {
            TransScope (context, exp->u.whilee.body);
        }

        return exp;
    }
    case A_forExp:
    {
        if (exp->u.forr.hi)
        {
            TransExp (context, exp->u.forr.hi);
        }

        if (exp->u.forr.lo)
        {
            TransExp (context, exp->u.forr.lo);
        }

        if (exp->u.forr.body)
        {
            TransScope (context, exp->u.forr.body);
        }

        return exp;
    }
    // TODO real asm parser is badly needed
    // TODO drop pseudo
    // TODO i need to be able use data address directly
    case A_macroCallExp:
    {
        struct String_t name = String (exp->u.macro.name->name);

        if (String_Equal (&name, "print"))
        {
            //TODO print!
            assert (0);
            //  la    $v0, L2
            //  lw    $v1, 0($v0)         # read string size
            //  addi  $v0, $v0, 0x04      # move string pointer to the first char
            //  add   $v1, $v1, $v0       # points to char position one greater than string len
            //  li    $t0, 0xffff0008     # transmitter control register
            //  li    $t1, 0xffff000c     # transmitter data register
            //print:
            //  beq   $v0, $v1, exit      # if string pointer is one greater than string lenght
            //wait:
            //  lw    $t2, 0($t0)         # load control register, it will be either 0x01 or 0x00
            //  beq   $t2, $zero, wait    # if it is zero we wait
            //  lbu   $t3, 0($v0)         # if it is one we ...
            //  sb    $t3, 0($t1)         # write one char
            //  addi  $v0, $v0, 0x01      # increment string pointer
            //  j     print               # repeat
            //exit:
            //
        }
        else if (String_Equal (&name, "println"))
        {
            //  la    $t0, L2
            //  lw    $t1, 0($t0)         # read string size
            //  addi  $t0, $t0, 0x04      # move string pointer to the first char
            //  add   $t1, $t1, $t0       # points to char position one greater than string len
            //  li    $t2, 0xffff0008     # transmitter control register
            //  li    $t3, 0xffff000c     # transmitter data register
            //wait:
            //  lw    $t4, 0($t2)         # load control register, it will be either 0x01 or 0x00
            //  beq   $t4, $zero, wait    # if it is zero we wait
            //  beq   $t0, $t1, exit      # if string pointer is one greater than string lenght
            //  lbu   $t5, 0($t0)         # if it is one we ...
            //  sb    $t5, 0($t3)         # write one char
            //  addi  $t0, $t0, 0x01      # increment string pointer
            //  j     wait                # repeat
            //exit:
            //  addi  $t0, $zero, 0x0A    # newline
            //  sb    $t0, 0($t3)         # write final char

            A_expList l = NULL;
            struct String_t buf = String ("");

            Temp_label l_wait = Temp_NewLabel();
            Temp_label l_exit = Temp_NewLabel();

            ASM ("add   `d0, `s0, `s1",
                 SL ("$t0", NULL), SL ("$zero", NULL), exp->u.macro.args->head->u.stringg);

            ASM ("lw    `d0, 0(`s0)",
                 SL ("$t1", NULL), SL ("$t0", NULL), NULL);

            ASM ("addi  `d0, `s0, 0x04",
                 SL ("$t0", NULL), SL ("$t0", NULL), NULL);

            ASM ("add   `d0, `s0, `s1",
                 SL ("$t1", NULL), SL ("$t1", SL ("$t0", NULL)), NULL);

            ASM ("li    `d0, 0xffff0008",
                 SL ("$t2", NULL), NULL, NULL);

            ASM ("li    `d0, 0xffff000c",
                 SL ("$t3", NULL), NULL, NULL);

            String_Init (&buf, l_wait->name);
            String_PushBack (&buf, ':');
            ASM (buf.data, NULL, NULL, NULL);

            ASM ("lw    `d0, 0(`s0)",
                 SL ("$t4", NULL), SL ("$t2", NULL), NULL);

            String_Init (&buf, "beq   `d0, `s0, ");
            String_Append (&buf, l_wait->name);
            ASM (buf.data, SL ("$t4", NULL), SL ("$zero", NULL), NULL);

            String_Init (&buf, "beq   `d0, `s0, ");
            String_Append (&buf, l_exit->name);
            ASM (buf.data, SL ("$t0", NULL), SL ("$t1", NULL), NULL);

            ASM ("lbu   `d0, 0(`s0)",
                 SL ("$t5", NULL), SL ("$t0", NULL), NULL);

            ASM ("sb    `d0, 0(`s0)",
                 SL ("$t5", NULL), SL ("$t3", NULL), NULL);

            ASM ("addi  `d0, `s0, 0x01",
                 SL ("$t0", NULL), SL ("$t0", NULL), NULL);

            String_Init (&buf, "j     ");
            String_Append (&buf, l_wait->name);
            ASM (buf.data, NULL, NULL, NULL);

            String_Init (&buf, l_exit->name);
            String_PushBack (&buf, ':');
            ASM (buf.data, NULL, NULL, NULL);

            ASM ("addi  `d0, `s0, 0x0A",
                 SL ("$t0", NULL), SL ("$zero", NULL), NULL);

            ASM ("sb    `d0, 0(`s0)",
                 SL ("$t0", NULL), SL ("$t3", NULL), NULL);

            // this makes the whole panic macro evaluate to 0(OK)
            LIST_PUSH (l, A_IntExp (&exp->loc, 0));

            exp->kind  = A_seqExp;
            exp->u.seq = l;

            return exp;
        }
        else if (String_Equal (&name, "panic"))
        {
            char * msg = checked_malloc (1024);
            sprintf (msg, "EXIT %d,%d 10000 %s",
                     exp->loc.first_line,
                     exp->loc.first_column,
                     exp->u.macro.args->head->u.stringg);

            A_exp println = A_MacroCallExp (
                                &exp->loc,
                                S_Symbol ("println"),
                                A_ExpList (A_StringExp (&exp->loc, msg), NULL));

            A_expList l = NULL;

            LIST_PUSH (l, TransExp (context, println));

            ASM ("li    `d0, 1", SL ("$a0", NULL), NULL, NULL);

            // exit program with status code 1
            ASM ("li    `d0, 17", SL ("$v0", NULL), NULL, NULL);

            ASM ("syscall",  NULL, SL ("$v0", SL ("$a0", NULL)), NULL);

            // this makes the whole panic macro evaluate to 0(OK)
            // FIXME must be stm
            LIST_PUSH (l, A_IntExp (&exp->loc, 0));

            exp->kind  = A_seqExp;
            exp->u.seq = l;

            return exp;
        }
        else if (String_Equal (&name, "assert"))
        {
            A_exp test = exp->u.macro.args->head;
            A_exp panic = A_MacroCallExp (
                              &test->loc,
                              S_Symbol ("panic"),
                              A_ExpList (A_StringExp (&test->loc, "Assert failed!"), NULL));

            A_scope fl = A_Scope (A_StmList (A_StmExp (TransExp (context, panic)), NULL));

            exp = A_IfExp (&test->loc, test, NULL, fl);

            // this makes the whole assert macro evaluate to 0(OK)
            A_exp zero = A_IntExp (&test->loc, 0);
            return A_SeqExp (&test->loc, A_ExpList (exp, A_ExpList (zero, NULL)));
        }
        else
        {
            assert (0);
        }
        break;
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
