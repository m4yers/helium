#include "ext/list.h"
#include "ext/vector.h"
#include "ext/str.h"

#include "error.h"
#include "mipsmachine.h"

#include "semant_mips.h"

// TODO finish the module

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

#define IS_IN_RANGE(value,left,right) (value >= left && value <= right)

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
        switch (*String_At (f, 0))
        {
        case SOURCE_REGISTER_5_BIT:
        case TARGET_REGISTER_5_BIT:
        case DESTINATION_REGISTER_5_BIT:
        case SAME_REGISTER_SOURCE_AND_TARGET_5_BIT:
        case SAME_REGISTER_SOURCE_AND_DESTINATION_5_BIT:
        case SAME_REGISTER_TARGET_AND_DESTINATION_5_BIT:
        {
            // at this point register name is already normalized
            if (op->kind != A_asmOpRegKind)
            {
                return String_New ("Expected Reg operand");
            }
            return NULL;
        }
        case ZERO_REGISTER:
        {
            // TODO check for $zero
            if (op->kind != A_asmOpRegKind)
            {
                return String_New ("Expected Reg operand");
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

static String TransOp (SemantMIPS_Context context, A_asmOp op)
{
    (void) context;

    if (op->kind == A_asmOpRegKind)
    {
        A_asmReg reg = op->u.reg;

        switch (reg->kind)
        {
        /*
         * Check if the name stands for the actual register
         */
        case A_asmRegNameKind:
        {
            // reg search looks up names as $name
            // TODO fix it
            struct String_t str = String ("$");
            String_Append (&str, reg->u.name);
            if (!F_RegistersGet_s (regs_all, str.data))
            {
                return String_New ("Unknown register");
            }
            break;
        }
        /*
         * Find register by its number and normalize the AST node to named register
         */
        case A_asmRegNumKind:
        {
            const char * name = F_RegistersGetName (regs_all, reg->u.num);
            if (!name)
            {
                return String_New ("Unknown register");
            }
            reg->kind = A_asmRegNameKind;
            reg->u.name = name;
            break;
        }
        }
    }
    /*
     * Same as for RegNum AST node
     */
    else if (op->kind == A_asmOpMemKind && op->u.mem.base->kind == A_asmRegNumKind)
    {
        const char * name = F_RegistersGetName (regs_all, op->u.mem.base->u.num);
        if (!name)
        {
            return String_New ("Unknown register");
        }
        op->u.mem.base->kind = A_asmRegNameKind;
        op->u.mem.base->u.name = name;
    }

    return NULL;
}

static void TransInst (SemantMIPS_Context context, A_asmStm stm)
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
        String r = TransOp (context, o);
        /*
         * If a single operand for some reason is wrong skip the line
         */
        if (r)
        {
            ERROR (&o->loc, 3101, String_Data (r), "")
            String_Delete (r);
            return;
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
                Vector_Push (&rejections, String_New ("Not enough operands"));
                opcode = NULL;
                break;
            }

            String r = OpMatchFormat (f, opList->head);
            if (r)
            {
                Vector_Push (&rejections, r);
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
            Vector_Push (&rejections, String_New ("Too many operands"));
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
                struct M_opCode_t * candidate = Vector_At (&candidates, i);
                String rejection = Vector_At (&rejections, i);
                sprintf (buf, "'%s' is rejected because '%s'",
                         candidate->format.data,
                         rejection->data);
                String_Append (s, buf);
            }

            ERROR (&stm->loc, 3102, s->data, inst->opcode);

            String_Delete (s);
            free (buf);
        }
        else
        {
            ERROR (&stm->loc, 3100, "Unknown opcode '%s'", inst->opcode);
        }
    }

    Vector_Fini (&candidates);
    VECTOR_FOREACH (String, s, &rejections)
    {
        String_Delete (*s);
    }
    Vector_Fini (&rejections);
}

static void TransStm (SemantMIPS_Context context, A_asmStm stm)
{
    switch (stm->kind)
    {
    case A_asmStmInstKind:
    {
        TransInst (context, stm);
    }
    }
}

int SemantMIPS_Translate (Program_Module m, A_asmStmList l)
{
    assert (m);
    assert (l);

    struct SemantMIPS_Context_t context;
    context.module = m;
    context.errors = 0;

    LIST_FOREACH (stm, l) TransStm (&context, stm);

    return context.errors == 0 ? 0 : 1;
}
