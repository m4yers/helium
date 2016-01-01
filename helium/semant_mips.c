#include "ext/list.h"
#include "ext/vector.h"
#include "ext/str.h"

#include "error.h"
#include "mipsmachine.h"

#include "semant_mips.h"

// TODO finish the module

#define ERROR(loc, code, format, ...)                                    \
{                                                                        \
    printf ("asm error line: %d\n", __LINE__);                           \
    Vector_PushBack(&context->module->errors.semant,                     \
            Error_New(loc, code, format, __VA_ARGS__));                  \
}                                                                        \

#define UINT_5_MIN   0
#define UINT_5_MAX   31
#define UINT_16_MIN  0
#define UINT_16_MAX  65535
#define INT_16_MIN  -32768
#define INT_16_MAX   32767

// TODO move it to mipsmachine
/*
 * This method tries to match and operand against opcode's format. It does not do any semantic
 * match like target vs source vs dest register, the register is just a register, any int value
 * is just an int value though sign and range are checked etc.
 */
static bool OpMatchFormat (const struct String_t * f, A_asmOp op)
{
    /* should not happen */
    if (String_Empty (f))
    {
        assert (0);
        return FALSE;
    }
    /*
     * If the format is 3-char long then it is a mem location e.g. o(b)
     */
    else if (String_Size (f) == 3)
    {
        if (op->kind != A_asmOpMemKind)
        {
            return FALSE;
        }
        // TODO validate further
        return TRUE;
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
            return op->kind == A_asmOpRegNameKind;
        }
        case ZERO_REGISTER:
        {
            // TODO check for $zero
            return op->kind == A_asmOpRegNameKind;
        }
        case SHIFT_AMOUNT_5_BIT:
        {
            return op->kind == A_asmOpIntKind
                   && op->u.integer >= UINT_5_MIN && op->u.integer <= UINT_5_MAX;
        }
        case UPPER_16_BITS_OF_ADDRESS_16_BIT:
        case UNSIGNED_IMMEDIATE_16_BIT:
        {
            return op->kind == A_asmOpIntKind
                   && op->u.integer >= UINT_16_MIN && op->u.integer <= UINT_16_MAX;
        }
        case SIGNED_IMMEDIATE_16_BIT:
        {
            return op->kind == A_asmOpIntKind
                   && op->u.integer >= INT_16_MIN && op->u.integer <= INT_16_MAX;
        }
        default:
        {
            assert (0);
        }
        }
    }

    /*
     * All other options are false
     */
    return FALSE;
}

// TODO validate and normalize reg names
static bool TransOp (SemantMIPS_Context context, A_asmOp op)
{
    (void) context;

    switch (op->kind)
    {
    case A_asmOpRegNameKind:
    {
        // reg search looks up names as $name
        // TODO fix it
        struct String_t str = String ("$");
        String_Append (&str, op->u.name);
        if (!F_RegistersGet_s (regs_all, str.data))
        {
            return FALSE;
        }
        break;
    }
    case A_asmOpRegNumKind:
    {
        const char * name = F_RegistersGetName (regs_all, op->u.num);
        if (!name)
        {
            return FALSE;
        }
        op->kind = A_asmOpRegNameKind;
        op->u.name = name;
        break;
    }
    case A_asmOpIntKind:
    {
        // need something to validate here?
        break;
    }
    case A_asmOpMemKind:
    {
        break;
    }
    }

    return TRUE;
}

static void TransInst (SemantMIPS_Context context, A_asmStm stm)
{
    A_asmStmInst inst = &stm->u.inst;

    /*
     * First we gonna collect all matched opcodes, there can be more than one signature.
     */
    struct Vector_t vec = Vector (const struct M_opCode_t *);

    for (size_t i = 0; i < mips_opcodes_num; ++i)
    {
        if (String_Equal (&mips_opcodes[i].name, inst->opcode))
        {
            Vector_PushBack (&vec, &mips_opcodes[i]);
        }
    }

    /*
     * Now we validate operands, or rather trying to find a matching signature
     */
    const struct M_opCode_t * opcode = NULL;

    LIST_FOREACH (o, inst->opList)
    {
        /*
         * If a single operand for some reason is wrong skip the line
         */
        if (!TransOp (context, o))
        {
            return;
        }
    }

    VECTOR_FOREACH (const struct M_opCode_t *, op, &vec)
    {
        opcode = *op;
        Vector format = String_Split (&opcode->format, ',');
        A_asmOpList opList = inst->opList;
        VECTOR_FOREACH (struct String_t, f, format)
        {
            /*
             * If not enough operands or operand is not correct
             */
            if (!opList || !OpMatchFormat (f, opList->head))
            {
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
            opcode = NULL;
        }
    }

    Vector_Fini (&vec);

    if (!opcode)
    {
        // TODO spill matched opcodes but for some reason(spill it too) unmatched signatures
        ERROR (&stm->loc, 3100, "Unknown opcode '%s'", inst->opcode);
    }
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
