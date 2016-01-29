#include <string.h>
#include <assert.h>

#include "util/hexspk.h"
#include "util/list.h"
#include "util/mem.h"

#include "modules/mips/machine.h"

#include "core/codegen.h"
#include "core/frame.h"
#include "core/temp.h"
#include "core/ir.h"

#define L Temp_TempList

#define R_INST(buffer, name)                                                            \
{                                                                                       \
    sprintf (buffer, "%-5s `d0, `s0, `s1", name);                                       \
}                                                                                       \

// TODO make sure the cast won't break anything
// C is 16 bit value so it should not
// instead we need to check the actual C value
#define I_INST(buffer, name, constant)                                                  \
{                                                                                       \
    const char * pattern = IS_HEX_SPEAK((unsigned int)constant)                         \
        ? "%-5s `d0, `s0, 0x%X"                                                         \
        : "%-5s `d0, `s0, %d";                                                          \
    sprintf (buffer, pattern, name, constant);                                          \
}                                                                                       \

static Temp_temp munchExp (T_exp e);
static void munchStm (T_stm s);

static ASM_lineList head = NULL, last = NULL;

static void emit (ASM_line line)
{
    if (last)
    {
        last = last->tail = ASM_LineList (line, NULL);
    }
    else
    {
        head = last = ASM_LineList (line, NULL);
    }
}

static Temp_tempList munchArgs (int index, T_expList list)
{
    if (!list)
    {
        return NULL;
    }

    char * buffer = checked_malloc (50);

    Temp_temp a;
    if (index < 4)
    {
        a = M_RegGet (regs_arguments, index);
        R_INST (buffer, "add");
        emit (ASM_Move (
                  buffer,
                  L (a, NULL),
                  L (munchExp (list->head),
                     L (zero,
                        NULL))));
    }
    else
    {
        // HMM do we really need a real temp here? mb just NULL for the rest?
        a = Temp_NewTemp();
        sprintf (buffer, "%-5s `s0, %d(`s1)", "sw", index * M_wordSize);
        emit (ASM_Oper (
                  buffer,
                  NULL,
                  L (munchExp (list->head),
                     L (F_SP(), NULL)),
                  NULL));
    }

    return L (a, munchArgs (index + 1, list->tail));
}

static const char * munchName (T_exp e)
{
    return e->u.NAME->name;
}

static Temp_temp munchExp (T_exp e)
{
    char * buffer = checked_malloc (100);

    switch (e->kind)
    {
    case T_MEM:
    {
        /*
         * TODO: add a flag to BINOP to quickly determine const arguments.
         */
        if (e->u.MEM->kind == T_BINOP
                && (e->u.MEM->u.BINOP.right->kind == T_CONST
                    || e->u.MEM->u.BINOP.left->kind == T_CONST))
        {
            T_exp ex;
            int co;

            if (e->u.MEM->u.BINOP.right->kind == T_CONST)
            {
                co = e->u.MEM->u.BINOP.right->u.CONST;
                ex = e->u.MEM->u.BINOP.left;
            }
            else
            {
                co = e->u.MEM->u.BINOP.left->u.CONST;
                ex = e->u.MEM->u.BINOP.right;
            }

            if (e->u.MEM->u.BINOP.op == T_minus)
            {
                co = -co;
            }

            Temp_temp r = Temp_NewTemp();
            sprintf (buffer, "%-5s `d0, %d(`s0)", "lw", co);
            emit (ASM_Oper (
                      buffer,
                      L (r, NULL),
                      L (munchExp (ex), NULL),
                      NULL));

            return r;

        }
        /*
         * probably wrong
         * TODO: how to read from a const MEM in MIPS?
         * FIXME use la pseudo for now
         * FIXME add check for C > 16bit value for all I instr
         */
        else if (e->u.MEM->kind == T_CONST)
        {
            Temp_temp r = Temp_NewTemp();
            sprintf (buffer, "%-5s `d0, %d", "lw", e->u.MEM->u.CONST);
            emit (ASM_Oper (
                      buffer,
                      L (r, NULL),
                      NULL,
                      NULL));

            return r;
        }
        else
        {
            Temp_temp r = Temp_NewTemp();
            sprintf (buffer, "%-5s `d0, 0(`s0)", "lw");
            emit (ASM_Oper (
                      buffer,
                      L (r, NULL),
                      L (munchExp (e->u.MEM), NULL),
                      NULL));

            return r;
        }
        assert (0);
    }
    case T_BINOP:
    {
        if ((e->u.BINOP.left->kind == T_CONST || e->u.BINOP.right->kind == T_CONST)
                && (e->u.BINOP.op == T_plus || e->u.BINOP.op == T_minus))
        {
            Temp_temp r = Temp_NewTemp();
            T_exp ex;
            int co;

            if (e->u.BINOP.op == T_plus)
            {
                if (e->u.BINOP.right->kind == T_CONST)
                {
                    co = e->u.BINOP.right->u.CONST;
                    ex = e->u.BINOP.left;
                }
                else
                {
                    co = e->u.BINOP.left->u.CONST;
                    ex = e->u.BINOP.right;
                }

                I_INST (buffer, "addi", co);
                emit (ASM_Oper (
                          buffer,
                          L (r, NULL),
                          L (munchExp (ex), NULL), NULL));
            }
            else
            {
                /*
                 * 1 - 2 -> -2 + 1
                 */
                if (e->u.BINOP.right->kind == T_CONST)
                {
                    co = e->u.BINOP.right->u.CONST;
                    ex = e->u.BINOP.left;

                    I_INST (buffer, "addi", -co);
                    emit (ASM_Oper (
                              buffer,
                              L (r, NULL),
                              L (munchExp (ex), NULL), NULL));
                }
                else
                {
                    R_INST (buffer, "sub");
                    emit (ASM_Oper (
                              buffer,
                              L (r, NULL),
                              L (munchExp (e->u.BINOP.left), L (munchExp (e->u.BINOP.right), NULL)),
                              NULL));
                }

            }

            return r;
        }
        else
        {
            Temp_temp r = Temp_NewTemp();
            switch (e->u.BINOP.op)
            {
            case T_mul:
            {
                sprintf (buffer, "%-5s `s0, `s1", "mult");
                emit (ASM_Oper (
                          buffer,
                          NULL,
                          L (munchExp (e->u.BINOP.left), L (munchExp (e->u.BINOP.right), NULL)),
                          NULL));

                char * buf = checked_malloc (30);
                sprintf (buf, "%-5s `d0", "mflo");
                emit (ASM_Oper (
                          buf,
                          L (r, NULL),
                          NULL,
                          NULL));
                break;
            }
            case T_div:
            {
                sprintf (buffer, "%-5s `s0, `s1", "div");
                emit (ASM_Oper (
                          buffer,
                          NULL,
                          L (munchExp (e->u.BINOP.left), L (munchExp (e->u.BINOP.right), NULL)),
                          NULL));

                char * buf = checked_malloc (30);
                sprintf (buf, "%-5s `d0", "mflo");
                emit (ASM_Oper (
                          buf,
                          L (r, NULL),
                          NULL,
                          NULL));
                break;
            }
            case T_plus:
            {
                R_INST (buffer, "add");
                emit (ASM_Oper (
                          buffer,
                          L (r, NULL),
                          L (munchExp (e->u.BINOP.left), L (munchExp (e->u.BINOP.right), NULL)),
                          NULL));
                break;
            }
            case T_minus:
            {
                R_INST (buffer, "sub");
                emit (ASM_Oper (
                          buffer,
                          L (r, NULL),
                          L (munchExp (e->u.BINOP.left), L (munchExp (e->u.BINOP.right), NULL)),
                          NULL));
                break;
            }
            default:
            {
                assert (0);
            }
            }

            return r;
        }
    }
    case T_CALL:
    {
        emit (ASM_MetaCallComment ("call-"));
        emit (ASM_MetaCallIn());
        Temp_temp r = v0;
        Temp_tempList args = munchArgs (0, e->u.CALL.args);
        sprintf (buffer, "%-5s %s", "jal", munchName (e->u.CALL.fun));
        emit (ASM_Oper (buffer,
                        L (r, L (ra, regs_caller_save->temps)),
                        args,
                        NULL));
        emit (ASM_MetaCallOut());
        return r;
    }
    case T_CONST:
    {
        I_INST (buffer, "addi", e->u.CONST);
        Temp_temp r = Temp_NewTemp();
        emit (ASM_Oper (buffer, L (r, NULL), L (zero, NULL), NULL));
        return r;
    }
    case T_TEMP:
    {
        return e->u.TEMP;
    }
    case T_NAME:
    {
        Temp_temp r = Temp_NewTemp();
        // FIXME drop the pseudo
        sprintf (buffer, "%-5s `d0, %s", "la", e->u.NAME->name);
        emit (ASM_Oper (buffer, L (r, NULL), NULL, NULL));
        return r;
    }
    default:
    {
        assert (0);
    }
    }
}

static void printAsmOpd (
    String out,
    ssize_t ord,
    IR_mipsOpd opd,
    T_stmList * pre,
    T_stmList * post,
    Temp_tempList * dsts,
    Temp_tempList * srcs,
    Temp_labelList * jmps)
{
    LIST_FOREACH (cli, opd->cls)
    {
        switch (cli->kind)
        {
        // FIXME remove cast after all IR becomes immutable
        case IR_mipsClosureHeStmPreKind:
        {
            LIST_PUSH (*pre, (T_stm)cli->u.heStmPre);
            break;
        }
        case IR_mipsClosureHeStmPostKind:
        {
            LIST_PUSH (*post, (T_stm)cli->u.heStmPost);
            break;
        }
        }
    }

    if (ord != -1)
    {
        if (ord != 0)
        {
            String_Append (out, ",");
        }

        String_Append (out, " ");
    }

    switch (opd->kind)
    {
    case IR_mipsOpdImmKind:
    {
        String_AppendF (out, "0x%02X", opd->u.imm.u.ival);
        break;
    }
    case IR_mipsOpdMemKind:
    {
        String_AppendF (out, "%jd(", opd->u.mem.offset);
        printAsmOpd (out, -1, opd->u.mem.base, pre, post, dsts, srcs, jmps);
        String_Append (out, ")");
        break;
    }
    case IR_mipsOpdLabKind:
    {
        String_AppendF (out, "%s", opd->u.lab.label->name);
        break;
    }
    case IR_mipsOpdRepKind:
    {
        switch (opd->u.rep.kind)
        {
        case A_asmOpUseDst:
        {
            LIST_PUSH (*dsts, opd->u.rep.u.tmp);
            String_AppendF (out, "`d%ju", LIST_SIZE (*dsts) - 1);
            break;
        }
        case A_asmOpUseSrc:
        {
            LIST_PUSH (*srcs, opd->u.rep.u.tmp);
            String_AppendF (out, "`s%ju", LIST_SIZE (*srcs) - 1);
            break;
        }
        case A_asmOpUseLab:
        {
            LIST_PUSH (*jmps, opd->u.rep.u.lab);
            String_AppendF (out, "`j%ju", LIST_SIZE (*jmps) - 1);
            break;
        }
        }
        break;
    }
    }
}

static void munchStm (T_stm s)
{
    char * buffer = checked_malloc (100);

    switch (s->kind)
    {
    case T_ASM:
    {
        struct String_t str;

        LIST_FOREACH (stm, s->u.ASSEMBLY.stms)
        {
            String_Init (&str, "");
            String_Reserve (&str, 128);

            switch (stm->kind)
            {
            case IR_mipsStmOpcKind:
            {
                String_AppendF (&str, "%-5s", stm->u.opc.spec->name.data);

                T_stmList pre = NULL;
                T_stmList post = NULL;

                Temp_tempList  dsts = NULL;
                Temp_tempList  srcs = NULL;
                Temp_labelList jmps = NULL;

                ssize_t ord = 0;

                LIST_FOREACH (opd, stm->u.opc.opdl)
                {
                    printAsmOpd (&str, ord++, opd, &pre, &post, &dsts, &srcs, &jmps);
                }

                LIST_FOREACH (pre_stm, pre)
                {
                    printf("pre\n");
                    munchStm (pre_stm);
                }

                emit (ASM_Oper (str.data, dsts, srcs, ASM_Targets (jmps)));

                LIST_FOREACH (post_stm, post)
                {
                    printf("post\n");
                    munchStm (post_stm);
                }

                break;
            }
            case IR_mipsStmLabKind:
            {
                String_AppendF (&str, "%s:", stm->u.lab.label->name);
                emit (ASM_Label (str.data, stm->u.lab.label));
                break;
            }
            default:
            {
                assert (0);
            }
            }
        }
        return;
    }
    case T_ASMOLD:
    {
        T_exp data = s->u.ASMOLD.data;
        Temp_tempList dl = s->u.ASMOLD.dst;
        Temp_tempList sl = s->u.ASMOLD.src;

        // the can be max 2 sources
        if (!sl && data)
        {
            sl = L (munchExp (data), NULL);
        }
        else if (data)
        {
            sl->tail = L (munchExp (data), NULL);
        }

        emit (ASM_Oper (s->u.ASMOLD.code, dl, sl, NULL));
        return;
    }
    case T_MOVE:
    {
        if (s->u.MOVE.dst->kind == T_MEM)
        {
            T_exp dst = s->u.MOVE.dst->u.MEM;

            if (dst->kind == T_BINOP
                    && (dst->u.BINOP.left->kind == T_CONST || dst->u.BINOP.right->kind == T_CONST))
            {
                T_exp exp;
                int c = 0;

                if (dst->u.BINOP.right->kind == T_CONST)
                {
                    c = dst->u.BINOP.right->u.CONST;
                    exp = dst->u.BINOP.left;
                }

                if (dst->u.BINOP.left->kind == T_CONST)
                {
                    c = dst->u.BINOP.left->u.CONST;
                    exp = dst->u.BINOP.right;
                }

                if (dst->u.BINOP.op == T_minus)
                {
                    c = -c;
                }

                sprintf (buffer, "%-5s `s0, %d(`s1)", "sw", c);
                emit (ASM_Oper (
                          buffer,
                          NULL,
                          L (munchExp (s->u.MOVE.src), L (munchExp (exp), NULL)),
                          NULL));
            }
            else if (dst->kind == T_BINOP)
                /* && dst->u.BINOP.left->kind == T_TEMP */
                /* && dst->u.BINOP.right->kind == T_TEMP) */
            {
                Temp_temp d = munchExp (dst);
                sprintf (buffer, "%-5s `s0, 0(`s1)", "sw");
                emit (ASM_Oper (
                          buffer,
                          NULL,
                          L (munchExp (s->u.MOVE.src),
                             L (d, NULL)), NULL));
            }
            // HMM should it exist?
            // T_MOVE(T_MEM(T_TEMP),T_***)
            else if (dst->kind == T_TEMP)
            {
                sprintf (buffer, "%-5s `s0, 0(`s1)", "--sw");
                emit (ASM_Oper (
                          buffer,
                          NULL,
                          L (munchExp (s->u.MOVE.src),
                             L (dst->u.TEMP, NULL)), NULL));
            }
            else if (s->u.MOVE.src->kind == T_MEM)
            {
                // TODO test this
                Temp_temp left = Temp_NewTemp();
                Temp_temp right = Temp_NewTemp();

                munchStm (T_Move (T_Temp (left), s->u.MOVE.dst));
                munchStm (T_Move (T_Temp (right), s->u.MOVE.src));
                munchStm (T_Move (T_Temp (left), T_Temp (right)));
            }
            else if (dst->kind == T_CONST)
            {
                Temp_temp addr = Temp_NewTemp();
                munchStm (T_Move (T_Temp (addr), T_Const (dst->u.CONST)));
                munchStm (T_Move (T_Temp (addr), s->u.MOVE.src));
            }
            else
            {
                sprintf (buffer, "%-5s `s0, 0(`s1)", "sw");
                emit (ASM_Oper (
                          buffer,
                          NULL,
                          L (munchExp (s->u.MOVE.src),
                             L (munchExp (s->u.MOVE.dst), NULL)), NULL));
            }
        }
        /*
         * Add constant value to a register
         * Temp(dst) = Temp(src) + C
         */
        else if (s->u.MOVE.dst->kind == T_TEMP
                 && s->u.MOVE.src->kind == T_BINOP
                 && s->u.MOVE.src->u.BINOP.left->kind == T_TEMP
                 && s->u.MOVE.src->u.BINOP.right->kind == T_CONST)
        {
            int value = s->u.MOVE.src->u.BINOP.right->u.CONST;
            if (s->u.MOVE.src->u.BINOP.op == T_minus)
            {
                value = -value;
            }
            I_INST (buffer, "addi", value);
            emit (ASM_Oper (
                      buffer,
                      L (s->u.MOVE.dst->u.TEMP, NULL),
                      L (s->u.MOVE.src->u.BINOP.left->u.TEMP,
                         NULL),
                      NULL));
        }
        else if (s->u.MOVE.dst->kind == T_TEMP
                 && s->u.MOVE.src->kind == T_MEM
                 && s->u.MOVE.src->u.MEM->kind == T_BINOP
                 && s->u.MOVE.src->u.MEM->u.BINOP.left->kind == T_TEMP
                 && s->u.MOVE.src->u.MEM->u.BINOP.right->kind == T_CONST)
        {
            int value = s->u.MOVE.src->u.MEM->u.BINOP.right->u.CONST;
            if (s->u.MOVE.src->u.MEM->u.BINOP.op == T_minus)
            {
                value = -value;
            }
            sprintf (buffer, "%-5s `d0, %d(`s0)", "lw", value);
            emit (ASM_Oper (
                      buffer,
                      L (s->u.MOVE.dst->u.TEMP, NULL),
                      L (s->u.MOVE.src->u.MEM->u.BINOP.left->u.TEMP,
                         NULL),
                      NULL));
        }
        else if (s->u.MOVE.dst->kind == T_TEMP
                 && s->u.MOVE.src->kind == T_TEMP)
        {
            R_INST (buffer, "add");
            emit (ASM_Move (
                      buffer,
                      L (s->u.MOVE.dst->u.TEMP, NULL),
                      L (s->u.MOVE.src->u.TEMP,
                         L (zero, NULL))));
        }
        else if (s->u.MOVE.dst->kind == T_TEMP
                 && s->u.MOVE.src->kind == T_CONST)
        {
            /* sprintf (buffer, "%-5s `d0, `s0, %#x", "addi", s->u.MOVE.src->u.CONST); */
            I_INST (buffer, "addi", s->u.MOVE.src->u.CONST);
            emit (ASM_Move (
                      buffer,
                      L (s->u.MOVE.dst->u.TEMP, NULL),
                      L (zero,
                         NULL)));
        }
        else if (s->u.MOVE.dst->kind == T_TEMP)
        {
            R_INST (buffer, "add");
            emit (ASM_Move (
                      buffer,
                      L (s->u.MOVE.dst->u.TEMP, NULL),
                      L (munchExp (s->u.MOVE.src),
                         L (zero, NULL))));
        }
        else
        {
            assert (0);
        }
        return;
    }
    case T_RJUMP:
    {
        sprintf (buffer, "%-5s `s0", "jr");
        emit (ASM_Oper (
                  buffer,
                  NULL,
                  L (s->u.RJUMP.src->u.TEMP, NULL),
                  NULL));
        return;
    }
    case T_JUMP:
    {
        sprintf (buffer, "%-5s `j0", "j");
        emit (ASM_Oper (
                  buffer,
                  NULL,
                  NULL,
                  ASM_Targets (Temp_LabelList (s->u.JUMP.exp->u.NAME, NULL))));
        return;
    }
    /*
     * TODO use T_notRel for subtrees that use constants either on the left or right sides
     */
    case T_CJUMP:
    {
        /* char * xor_buf = checked_malloc (100); */
        /* Temp_temp flag = Temp_NewTemp(); */

        switch (s->u.CJUMP.op)
        {
        case T_eq:
        {
            Temp_temp left = munchExp (s->u.CJUMP.left);
            Temp_temp right;
            if (s->u.CJUMP.right->kind == T_CONST && s->u.CJUMP.right->u.CONST == 0)
            {
                right = zero;
            }
            else
            {
                right = munchExp (s->u.CJUMP.right);
            }

            sprintf (buffer, "%-5s `s0, `s1, `j0", "beq");
            emit (ASM_Oper (
                      buffer,
                      NULL,
                      L (left, L (right, NULL)),
                      ASM_Targets (
                          Temp_LabelList (s->u.CJUMP.true,
                                          Temp_LabelList (s->u.CJUMP.false, NULL)))));
            break;
        }
        case T_ne:
        {
            Temp_temp left = munchExp (s->u.CJUMP.left);
            Temp_temp right;
            if (s->u.CJUMP.right->kind == T_CONST && s->u.CJUMP.right->u.CONST == 0)
            {
                right = zero;
            }
            else
            {
                right = munchExp (s->u.CJUMP.right);
            }

            sprintf (buffer, "%-5s `s0, `s1, `j0", "bne");
            emit (ASM_Oper (
                      buffer,
                      NULL,
                      L (left, L (right, NULL)),
                      ASM_Targets (
                          Temp_LabelList (s->u.CJUMP.true,
                                          Temp_LabelList (s->u.CJUMP.false, NULL)))));
            break;

        }
        // TODO in cases where left is non-const and right is const we need to reverse the op
        // blt $l,$r,Label ->
        // slt $t,$l,$r;
        // bne $t,$zero,Label
        case T_lt:
        {
            Temp_temp r = Temp_NewTemp();
            Temp_temp left = munchExp (s->u.CJUMP.left);
            Temp_temp right;
            char * buf = checked_malloc (30);
            if (s->u.CJUMP.right->kind == T_CONST)
            {
                if (s->u.CJUMP.right->u.CONST == 0)
                {
                    right = zero;
                    R_INST (buf, "slt");
                    emit (ASM_Oper (
                              buf,
                              L (r, NULL),
                              L (left, L (right, NULL)),
                              NULL));
                }
                else
                {
                    I_INST (buf, "slti", s->u.CJUMP.right->u.CONST);
                    emit (ASM_Oper (
                              buf,
                              L (r, NULL),
                              L (left, NULL),
                              NULL));
                }
            }
            else
            {
                // TODO test non const variatns
                right = munchExp (s->u.CJUMP.right);
                R_INST (buf, "slt");
                emit (ASM_Oper (
                          buf,
                          L (r, NULL),
                          L (left, L (right, NULL)),
                          NULL));
            }

            sprintf (buffer, "%-5s `s0, `s1, `j0", "bne");
            emit (ASM_Oper (
                      buffer,
                      NULL,
                      L (r, L (zero, NULL)),
                      ASM_Targets (
                          Temp_LabelList (s->u.CJUMP.true,
                                          Temp_LabelList (s->u.CJUMP.false, NULL)))));
            break;
        }
        // ble $l,$r,Label ->
        // slt $t,$r,$l;
        // beq $t,$zero,Label
        case T_le:
        {
            /*
             * The evaluation order is reversed here, since MIPS has only slt and slti
             * instructions and in case of GT op we are concerned whether the left
             * operand is a const
             *
             * TODO in cases where left is non-const and right is const we need to reverse the op
             */
            Temp_temp r = Temp_NewTemp();
            Temp_temp left;
            Temp_temp right = munchExp (s->u.CJUMP.right);
            char * buf = checked_malloc (30);
            if (s->u.CJUMP.left->kind == T_CONST)
            {
                if (s->u.CJUMP.left->u.CONST == 0)
                {
                    left = zero;
                    R_INST (buf, "slt");
                    emit (ASM_Oper (
                              buf,
                              L (r, NULL),
                              L (right, L (left, NULL)),
                              NULL));
                }
                else
                {
                    I_INST (buf, "slti", s->u.CJUMP.left->u.CONST);
                    emit (ASM_Oper (
                              buf,
                              L (r, NULL),
                              L (right, NULL),
                              NULL));
                }
            }
            else
            {
                // TODO test non const variatns
                left = munchExp (s->u.CJUMP.left);
                R_INST (buf, "slt");
                emit (ASM_Oper (
                          buf,
                          L (r, NULL),
                          L (right, L (left, NULL)),
                          NULL));
            }

            sprintf (buffer, "%-5s `s0, `s1, `j0", "beq");
            emit (ASM_Oper (
                      buffer,
                      NULL,
                      L (r, L (zero, NULL)),
                      ASM_Targets (
                          Temp_LabelList (s->u.CJUMP.true,
                                          Temp_LabelList (s->u.CJUMP.false, NULL)))));
            break;

        }
        // bgt $l,$r,Label ->
        // slt $t,$r,$l;
        // bne $t,$zero,Label
        case T_gt:
        {
            /*
             * The evaluation order is reversed here, since MIPS has only slt and slti
             * instructions and in case of GT op we are concerned whether the left
             * operand is a const
             *
             * TODO in cases where left is non-const and right is const we need to reverse the op
             */
            Temp_temp r = Temp_NewTemp();
            Temp_temp left;
            Temp_temp right = munchExp (s->u.CJUMP.right);
            char * buf = checked_malloc (30);
            if (s->u.CJUMP.left->kind == T_CONST)
            {
                if (s->u.CJUMP.left->u.CONST == 0)
                {
                    left = zero;
                    R_INST (buf, "slt");
                    emit (ASM_Oper (
                              buf,
                              L (r, NULL),
                              L (right, L (left, NULL)),
                              NULL));
                }
                else
                {
                    I_INST (buf, "slti", s->u.CJUMP.left->u.CONST);
                    emit (ASM_Oper (
                              buf,
                              L (r, NULL),
                              L (right, NULL),
                              NULL));
                }
            }
            else
            {
                // TODO test non const variatns
                left = munchExp (s->u.CJUMP.left);
                R_INST (buf, "slt");
                emit (ASM_Oper (
                          buf,
                          L (r, NULL),
                          L (right, L (left, NULL)),
                          NULL));
            }

            sprintf (buffer, "%-5s `s0, `s1, `j0", "bne");
            emit (ASM_Oper (
                      buffer,
                      NULL,
                      L (r, L (zero, NULL)),
                      ASM_Targets (
                          Temp_LabelList (s->u.CJUMP.true,
                                          Temp_LabelList (s->u.CJUMP.false, NULL)))));
            break;
        }
        // bge $l,$r,Label ->
        // slt $t,$l,$r;
        // beq $t,$zero,Label
        case T_ge:
        {
            Temp_temp r = Temp_NewTemp();
            Temp_temp left = munchExp (s->u.CJUMP.left);
            Temp_temp right;
            char * buf = checked_malloc (30);
            if (s->u.CJUMP.right->kind == T_CONST)
            {
                if (s->u.CJUMP.right->u.CONST == 0)
                {
                    right = zero;
                    R_INST (buf, "slt");
                    emit (ASM_Oper (
                              buf,
                              L (r, NULL),
                              L (left, L (right, NULL)),
                              NULL));
                }
                else
                {
                    I_INST (buf, "slti", s->u.CJUMP.right->u.CONST);
                    emit (ASM_Oper (
                              buf,
                              L (r, NULL),
                              L (left, NULL),
                              NULL));
                }
            }
            else
            {
                // TODO test non const variatns
                right = munchExp (s->u.CJUMP.right);
                R_INST (buf, "slt");
                emit (ASM_Oper (
                          buf,
                          L (r, NULL),
                          L (left, L (right, NULL)),
                          NULL));
            }

            sprintf (buffer, "%-5s `s0, `s1, `j0", "beq");
            emit (ASM_Oper (
                      buffer,
                      NULL,
                      L (r, L (zero, NULL)),
                      ASM_Targets (
                          Temp_LabelList (s->u.CJUMP.true,
                                          Temp_LabelList (s->u.CJUMP.false, NULL)))));
            break;
        }
        default:
        {
            assert (FALSE);
        }
        }
        return;
    }
    case T_LABEL:
    {
        sprintf (buffer, "%s:", s->u.LABEL->name);
        emit (ASM_Label (buffer, s->u.LABEL));
        return;
    }
    case T_EXP:
    {
        munchExp (s->u.EXP);
        return;
    }
    /*
     * li $a0, RESULT
     * li $v0, 17
     * syscall
     */
    case T_EXIT:
    {
        R_INST (buffer, "add");
        emit (ASM_Move (
                  buffer,
                  L (a0, NULL),
                  L (munchExp (s->u.EXIT), L (zero, NULL))));

        buffer = checked_malloc (50);
        // TODO move out syscall constants
        I_INST (buffer, "addi", 17);
        emit (ASM_Oper (
                  buffer,
                  L (v0, NULL),
                  L (zero, NULL),
                  NULL));

        emit (ASM_Oper (
                  "syscall",
                  NULL,
                  L (a0, L (v0, NULL)),
                  NULL));

        return;
    }
    case T_COMMENT:
    {
        emit (ASM_MetaCallComment (s->u.COMMENT));
        return;
    }
    default:
    {
        assert (0);
    }
    }
}

ASM_lineList F_CodeGen (F_frame frame, T_stmList list)
{
    (void) frame;

    ASM_lineList r;

    LIST_FOREACH (stm, list) munchStm (stm);

    r = head;
    head = last = NULL;
    return r;
}
