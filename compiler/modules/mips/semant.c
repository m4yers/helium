#include "util/list.h"
#include "util/vector.h"
#include "util/str.h"

#include "core/error.h"

#include "modules/helium/translate.h"
#include "modules/mips/machine.h"
#include "modules/mips/semant.h"

/*
 * 3100 Unknown opcode
 * 3101 Bad operand(unknown register etc)
 * 3102 Could not match opcode signature
 */
#define ERROR(loc, code, format, ...)                                    \
{                                                                        \
    printf ("asm semant error line: %d\n", __LINE__);                    \
    Vector_PushBack(&context->module->errors.semant,                     \
            Error_New(loc, code, format, __VA_ARGS__));                  \
}                                                                        \

#define UINT_5_MIN   0
#define UINT_5_MAX   31
#define UINT_16_MIN  0
#define UINT_16_MAX  65535
#define INT_16_MIN  -32768
#define INT_16_MAX   32767
#define UINT_20_MAX  1048575
#define UINT_26_MAX  67108863

#define IS_IN_RANGE(value,left,right) ((value) >= (left) && (value) <= (right))

/*
 * This method tries to match and operand against opcode's format. It does not do any semantic
 * match like target vs source vs dest register, the register is just a register, any int value
 * is just an int value though sign and range are checked etc. Also it does not validate insts for
 * a specific ISA(MIPS I,II,III ect) yet.
 *
 * @format  A format to match against
 * @operand An operand to validate
 * @return  An error String if there was one, NULL if the match was a success.
 */
static String OpMatchFormat (const struct String_t * f, A_asmOp op)
{
    /* should not happen */
    if (String_Empty (f))
    {
        assert (0);
        return String_New ("No format provided");
    }
    /*
     * If the format is 4-char long then it is a mem location e.g. o(b)
     */
    else if (String_Size (f) == 4)
    {
        if (op->kind != A_asmOpMemKind)
        {
            return String_New ("Expected Mem operand");
        }
        switch (*String_At (f, 0))
        {
        case SIGNED_OFFSET_16_BIT:
        {
            if (!IS_IN_RANGE (op->u.mem.offset, INT_16_MIN, INT_16_MAX))
            {
                return String_New (
                           "Signed offset must be a 16-bit value in range from -32,768 to 32,767");
            }
            return NULL;

        }
        // not yet implemented, and should i?
        case MA_GENERAL_EXPRESSION_32_BIT:
        {
            return String_New ("Expected a general 32-bit expression");
        }
        // If happens you extend the cases
        default:
        {
            assert (0);
        }
        }
    }
    /*
     * If the format is 1-char long the it is a register or an immediate value
     */
    else if (String_Size (f) == 1)
    {
        char l = *String_At (f, 0);
        switch (l)
        {
        case SOURCE_REGISTER_5_BIT:
        case TARGET_REGISTER_5_BIT:
        case DESTINATION_REGISTER_5_BIT:
        case SAME_REGISTER_SOURCE_AND_TARGET_5_BIT:
        case SAME_REGISTER_SOURCE_AND_DESTINATION_5_BIT:
        case SAME_REGISTER_TARGET_AND_DESTINATION_5_BIT:
        {
            // at this point register name is already normalized
            if (op->kind != A_asmOpRegKind
                    /*
                     * Replacements and Variables gonna be resolved as a register(temp)
                     */
                    && op->kind != A_asmOpRepKind
                    && op->kind != A_asmOpVarKind)
            {
                return String_New ("Expected Reg operand");
            }
            return NULL;
        }
        case ZERO_REGISTER:
        {
            if (op->kind != A_asmOpRegKind)
            {
                return String_New ("Expected $zero register operand");
            }

            Temp_temp r = M_RegGet (regs_all, op->u.reg->u.name);
            if (r != zero)
            {
                return String_New ("Expected $zero register operand");
            }
            return NULL;
        }
        case SHIFT_AMOUNT_5_BIT:
        {
            if (op->kind != A_asmOpIntKind)
            {
                return String_New ("Expected Const operand");
            }

            if (!IS_IN_RANGE (op->u.integer , UINT_5_MIN , UINT_5_MAX))
            {
                return String_New (
                           "Shift amount must be a 5-bit value in range from 0 to 31");
            }

            return NULL;
        }
        case UPPER_16_BITS_OF_ADDRESS_16_BIT:
        case UNSIGNED_IMMEDIATE_16_BIT:
        {
            if (op->kind != A_asmOpIntKind)
            {
                return String_New ("Expected Const operand");
            }

            if (!IS_IN_RANGE (op->u.integer , UINT_16_MIN , UINT_16_MAX))
            {
                return String_New (
                           "Constant must be a 16-bit value in range from 0 to 65535");
            }

            return NULL;

        }
        case SIGNED_IMMEDIATE_16_BIT:
        {
            if (op->kind != A_asmOpIntKind)
            {
                return String_New ("Expected Const operand");
            }

            if (!IS_IN_RANGE (op->u.integer, INT_16_MIN, INT_16_MAX))
            {
                return String_New (
                           "Constant must be a 16-bit value in range from -32,768 to 32,767");
            }

            return NULL;
        }
        case SYSCALL_FUNCTION_CODE_20_BIT:
        {
            if (op->kind != A_asmOpIntKind)
            {
                return String_New ("Expected Const operand");
            }

            if (!IS_IN_RANGE (op->u.integer, 0, UINT_20_MAX))
            {
                return String_New (
                           "Syscall function code must be a 20-bit value in range\
   from 0 to 1,048,575");
            }

            return NULL;
        }
        case TARGET_ADDRESS_26_BIT:
        {
            if (op->kind != A_asmOpIntKind)
            {
                return String_New ("Expected Const operand");
            }

            if (!IS_IN_RANGE (op->u.integer, 0, UINT_26_MAX))
            {

                return String_New (
                           "Target address must be a 20-bit value in range from 0 to 67,108,863");
            }
        }
        default:
        {
            assert (0);
        }
        }
    }

    return NULL;
}

static String NormalizeReg (A_asmReg reg)
{
    switch (reg->kind)
    {
    /*
     * Check if the name stands for the actual register
     */
    case A_asmRegNameKind:
    {
        if (!M_RegGet (regs_all, reg->u.name))
        {
            return String_New ("Unknown register name");
        }
        break;
    }
    /*
     * Find register by its number and normalize the AST node to named register
     */
    case A_asmRegNumKind:
    {
        if (!IS_IN_RANGE (reg->u.num, 0, regs_all->number - 1))
        {
            return String_New ("Unknown register");
        }
        reg->kind = A_asmRegNameKind;
        reg->u.name = M_RegGetName (regs_all, reg->u.num);
        break;
    }
    }

    return NULL;
}

/*
 * Method tries to normalize an operand, e.g. numeric registers are converted to a named form,
 * making sure the reister exists at all.
 */
static String NormalizeOp (A_asmOp op)
{
    if (op->kind == A_asmOpRegKind)
    {
        return NormalizeReg(op->u.reg);
    }
    else if (op->kind == A_asmOpMemKind)
    {
        /*
         * We handle only register base, other things like variable interpolation is handled
         * later
         */
        if (op->u.mem.base->kind == A_asmRegNumKind)
        {
            return NormalizeReg(op->u.mem.base->u.reg);
        }
    }

    return NULL;
}

static const struct M_opCode_t * FindInst (A_asmStm stm, struct Error_t * err)
{
    A_asmStmInst inst = &stm->u.inst;

    /*
     * First we gonna collect all matched opcodes, there can be more than one signature.
     */
    struct Vector_t candidates = Vector (const struct M_opCode_t *);

    for (size_t i = 0; i < mips_opcodes_num; ++i)
    {
        if (String_Equal (&mips_opcodes[i].name, inst->opcode))
        {
            Vector_PushBack (&candidates, &mips_opcodes[i]);
        }
    }

    /*
     * Now we validate operands, or rather trying to find a matching signature
     */
    const struct M_opCode_t * opcode = NULL;

    LIST_FOREACH (o, inst->opList)
    {
        String r = NormalizeOp (o);
        /*
         * If a single operand for some reason is wrong skip the line
         */
        if (r)
        {
            *err = Error_New (&o->loc, 3101, String_Data (r), "");
            return NULL;
        }
    }

    // this var stores rejection explanations for the opcodes candidates
    struct Vector_t rejections = Vector (String);

    VECTOR_FOREACH (const struct M_opCode_t *, ca, &candidates)
    {
        opcode = *ca;

        Vector format = String_Split (&opcode->format, ',');

        A_asmOpList opList = inst->opList;

        VECTOR_FOREACH (struct String_t, f, format)
        {
            if (!opList)
            {
                Vector_PushBack (&rejections, String_New ("Not enough operands"));
                opcode = NULL;
                break;
            }

            String r = OpMatchFormat (f, opList->head);
            if (r)
            {
                Vector_PushBack (&rejections, r);
                opcode = NULL;
                break;
            }

            opList = LIST_NEXT (opList);
        }

        /*
         * We found almost correct opcode but there are to much operands
         */
        if (opcode && opList)
        {
            Vector_PushBack (&rejections, String_New ("Too many operands"));
            opcode = NULL;
        }

        /*
         * We have found what we needed
         */
        if (opcode)
        {
            break;
        }
    }

    if (!opcode)
    {
        if (Vector_Size (&rejections))
        {
            String s = String_New (
                           "Could not find proper opcode signature, here are some candidates:");
            // FIXME do not use malloc
            char * buf = checked_malloc (1024);

            for (size_t i = 0; i < Vector_Size (&rejections); ++i)
            {
                struct M_opCode_t * candidate = * (M_opCode *)Vector_At (&candidates, i);
                String rejection = * (String *)Vector_At (&rejections, i);
                sprintf (buf, "\n'%s %s' is rejected because '%s'",
                         candidate->name.data,
                         candidate->format.data,
                         rejection->data);
                String_Append (s, buf);
            }

            *err = Error_New (&stm->loc, 3102, s->data, inst->opcode);
            return NULL;

            String_Delete (s);
            free (buf);
        }
        else
        {
            *err = Error_New (&stm->loc, 3100, "Unknown opcode '%s'", inst->opcode);
            return NULL;
        }
    }

    Vector_Fini (&candidates);
    VECTOR_FOREACH (String, s, &rejections)
    {
        String_Delete (*s);
    }
    Vector_Fini (&rejections);

    *err = Error_OK;
    return opcode;
}

static A_asmStmList TransInst (Sema_MIPSContext context, A_asmStm stm)
{
    struct Error_t err;
    const struct M_opCode_t * opcode = FindInst (stm, &err);
    if (err.code != 0)
    {
        Vector_PushBack (&context->module->errors.semant, err);
        return NULL;
    }

    A_asmStmList pre = NULL;
    A_asmStmList post = NULL;
    Vector format = String_Split (&opcode->format, ',');

    A_asmStmInst inst = &stm->u.inst;
    A_asmOpList opList = inst->opList;

    /*
     * Now we need to actually parse the instruction and replace all registers occurances with
     * replacements fields like `s0, `s1, `d0 etc, and push all the registers into .src/.dst lists
     * of the top level ASM STM AST node
     * FIXME it is a double work to parse format twice, can you optimize it somehow?
     */

    VECTOR_FOREACH (struct String_t, f, format)
    {
        A_asmOp op = opList->head;

        if (String_Size (f) == 4)
        {
            if (op->u.mem.base->kind == A_asmOpRegKind)
            {
                Temp_temp r = M_RegGet (regs_all, op->u.mem.base->u.reg->u.name);
                LIST_PUSH (stm->src, Tr_UnEx (Tr_Temp (r)));
                op->u.mem.base->kind = A_asmOpRepKind;
                op->u.mem.base->u.rep.use = A_asmOpUseSrc;
                op->u.mem.base->u.rep.pos = LIST_SIZE (stm->src) - 1;
            }
            // TODO lvalue
            else
            {
                assert (0);
            }
        }
        else if (String_Size (f) == 1)
        {
            char l = *String_At (f, 0);
            switch (l)
            {
            case TARGET_REGISTER_5_BIT:
            case DESTINATION_REGISTER_5_BIT:
            {
                if (op->kind == A_asmOpRegKind && op->u.reg->kind == A_asmRegNameKind)
                {
                    Temp_temp r = M_RegGet (regs_all, op->u.reg->u.name);
                    /*
                     * TODO
                     * this check here because of 't' register that can be used as 'd' in the
                     * beginning of the format string, this is a bit strange imho, you need to read
                     * more carefully through binutils sources
                     */
                    if (__i_f == 0)
                    {
                        LIST_PUSH (stm->dst, Tr_UnEx (Tr_Temp (r)));
                        op->kind = A_asmOpRepKind;
                        op->u.rep.use = A_asmOpUseDst;
                        op->u.rep.pos = LIST_SIZE (stm->dst) - 1;
                    }
                    else
                    {
                        LIST_PUSH (stm->src, Tr_UnEx (Tr_Temp (r)))
                        op->kind = A_asmOpRepKind;
                        op->u.rep.use = A_asmOpUseSrc;
                        op->u.rep.pos = LIST_SIZE (stm->src) - 1;
                    }
                }
                else if (op->kind == A_asmOpVarKind)
                {
                    if (__i_f == 0)
                    {
                        Temp_temp t = Temp_NewTemp();
                        Sema_Exp sexp = Sema_TransVar (context->context, op->u.var, TRUE);
                        LIST_PUSH (stm->post, T_Move (Tr_UnEx (sexp.exp), T_Temp (t)));
                        LIST_PUSH (stm->dst, T_Temp (t));
                        op->kind = A_asmOpRepKind;
                        op->u.rep.use = A_asmOpUseDst;
                        op->u.rep.pos = LIST_SIZE (stm->dst) - 1;
                    }
                    else
                    {
                        Sema_Exp sexp = Sema_TransVar (context->context, op->u.var, TRUE);
                        LIST_PUSH (stm->src, Tr_UnEx (sexp.exp));
                        op->kind = A_asmOpRepKind;
                        op->u.rep.use = A_asmOpUseSrc;
                        op->u.rep.pos = LIST_SIZE (stm->src) - 1;
                    }
                }
                else
                {
                    assert (0);
                }
                break;
            }
            case SOURCE_REGISTER_5_BIT:
            case SAME_REGISTER_SOURCE_AND_TARGET_5_BIT:
            case SAME_REGISTER_SOURCE_AND_DESTINATION_5_BIT:
            case SAME_REGISTER_TARGET_AND_DESTINATION_5_BIT:
            {
                if (op->kind == A_asmOpRegKind && op->u.reg->kind == A_asmRegNameKind)
                {
                    Temp_temp r = M_RegGet (regs_all, op->u.reg->u.name);
                    LIST_PUSH (stm->src, Tr_UnEx (Tr_Temp (r)));
                    op->kind = A_asmOpRepKind;
                    op->u.rep.use = A_asmOpUseSrc;
                    op->u.rep.pos = LIST_SIZE (stm->src) - 1;
                }
                else if (op->kind == A_asmOpVarKind)
                {
                    Sema_Exp sexp = Sema_TransVar (context->context, op->u.var, TRUE);
                    LIST_PUSH (stm->src, Tr_UnEx (sexp.exp));
                    op->kind = A_asmOpRepKind;
                    op->u.rep.use = A_asmOpUseSrc;
                    op->u.rep.pos = LIST_SIZE (stm->src) - 1;
                }
                else
                {
                    assert (0);
                }
                break;
            }
            // immediate go directly into asm string
            default:
            {
            }
            }
        }
        opList = LIST_NEXT (opList);
    }

    A_asmStmList result = NULL;

    if (pre)
    {
        LIST_JOIN (result, pre);
    }

    LIST_JOIN (result, A_AsmStmList (stm, NULL));

    if (post)
    {
        LIST_JOIN (result, post);
    }

    return result;
}

static A_asmStmList TransStm (Sema_MIPSContext context, A_asmStm stm)
{
    switch (stm->kind)
    {
    case A_asmStmInstKind:
    {
        return TransInst (context, stm);
    }
    default:
    {
        return A_AsmStmList (stm, NULL);
    }
    }
}

A_asmStmList SemantMIPS_Translate (Sema_Context c, struct A_asmDec_t * d)
{
    assert (c);
    assert (d);

    struct Sema_MIPSContext_t context;
    context.dec = d;
    context.context = c;
    context.module = c->module;
    context.errors = 0;

    A_asmStmList result = NULL;

    LIST_FOREACH (stm, (A_asmStmList)d->code)
    {
        A_asmStmList stml = TransStm (&context, stm);
        if (stml)
        {
            LIST_JOIN (result, stml);
        }
    }

    return result;
}
