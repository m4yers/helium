#include "ext/list.h"
#include "ext/vector.h"
#include "ext/str.h"

#include "error.h"
#include "mipsmachine.h"

#include "semant_mips.h"

#define ERROR(loc, code, format, ...)                                    \
    {                                                                    \
        printf ("asm error line: %d\n", __LINE__);                       \
        Vector_PushBack(&context->module->errors.semant,                 \
                Error_New(loc, code, format, __VA_ARGS__));              \
    }                                                                    \

static bool IsOpCorrect (const struct String_t * f, A_asmOp op)
{
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
            if (!opList || !IsOpCorrect (f, opList->head))
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
