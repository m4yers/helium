#include <stdint.h>

#include "util/util.h"
#include "util/list.h"
#include "util/vector.h"
#include "util/str.h"

#include "core/error.h"
#include "core/asm.h"

#include "modules/helium/translate.h"
#include "modules/mips/machine.h"
#include "modules/mips/semant.h"
#include "modules/mips/ir.h"

/*
 * 3100 Unknown opcode
 * 3101 Bad operand(unknown register etc)
 * 3102 Could not match opcode signature
 */
#define ERROR(loc, code, format, ...)                                    \
{                                                                        \
    printf ("asm semant error line: %d\n", __LINE__);                    \
    Vector_PushBack(&cntx->module->errors.semant,                        \
            Error_New(loc, code, format, __VA_ARGS__));                  \
}                                                                        \

#define UINT5_MAX  UINTMAX_C( 31)
#define UINT20_MAX UINTMAX_C( 1048575)
#define INT26_MIN  INTMAX_C(-33554432)
#define INT26_MAX  INTMAX_C( 33554431)

#define IS_IN_RANGE(value,left,right) ((value) >= (left) && (value) <= (right))

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
static String NormalizeOp (A_asmOp opd)
{
    switch (opd->kind)
    {
    case A_asmOpRepKind:
    {
        break;
    }
    case A_asmOpVarKind:
    {
        break;
    }
    case A_asmOpLitKind:
    {
        break;
    }
    case A_asmOpRegKind:
    {
        return NormalizeReg (opd->u.reg);
        break;
    }
    case A_asmOpTmpKind:
    {
        break;
    }
    case A_asmOpLabKind:
    {
        break;
    }
    case A_asmOpMemKind:
    {
        if (opd->u.mem.base->kind == A_asmRegNumKind)
        {
            return NormalizeReg (opd->u.reg);
        }
        break;
    }
    }

    return NULL;
}

static bool PreProcess (Sema_mipsContext cntx, struct A_asmDec_t * dec)
{
    bool result = TRUE;

    LIST_FOREACH (stm, dec->code)
    {
        switch (stm->kind)
        {
        case A_asmStmInstKind:
        {
            A_asmStmInst opc = &stm->u.inst;
            LIST_FOREACH (opd, opc->opList)
            {
                String str = NormalizeOp (opd);
                if (str)
                {
                    /*
                     * From this point the result of pre-processing is corrupt but we still proceed
                     * to process all the lines for convenience sake.
                     */
                    ERROR (&opd->loc, 3101, "%s", str->data);
                    result = FALSE;
                }
            }
        }
        case A_asmStmLabKind:
        {
            /*
             * If the label is a meta label we need generate the name for the label and save it
             * for further occurrences of the meta label
             */
            if (stm->u.lab.meta)
            {
                S_symbol sym = stm->u.lab.sym;
                Temp_label lab = (Temp_label)TAB_Look (cntx->meta_labs, sym);
                if (!lab)
                {
                    lab = Temp_NewLabel();
                    TAB_Enter (cntx->meta_labs, sym, lab);
                }
            }
            /*
             * Otherwise we create a label with specified name and save it but to a different map
             */
            else
            {
                S_symbol sym = stm->u.lab.sym;
                Temp_label lab = (Temp_label)TAB_Look (cntx->norm_labs, sym);
                if (!lab)
                {
                    // TODO there must be a global translation unit context that holds all labels
                    // TODO and we need to make sure the label is unique
                    lab = Temp_NamedLabel (sym->name);
                    TAB_Enter (cntx->norm_labs, sym, lab);
                }
            }
        }
        }
    }

    return result;
}

#define IS_DST(c,p) p == 0 && (c == TARGET_REGISTER_5_BIT || c == DESTINATION_REGISTER_5_BIT)

static IR_mipsOpd TrOpd (String * err, Sema_mipsContext cntx, A_asmOp opd, String frmt, size_t pos)
{
    IR_mipsOpd iropd = NULL;

    *err = NULL;

    /* should not happen */
    if (String_Empty (frmt))
    {
        assert (0);
        *err =  String_New ("No format provided");
        return NULL;
    }
    /*
     * If the format is 4-char long then it is a mem location e.g. o(b)
     */
    else if (String_Size (frmt) == 4)
    {
        if (opd->kind != A_asmOpMemKind)
        {
            *err = String_New ("Expected Mem operand");
            return NULL;
        }

        /*
         * We need to match offset type against passed AST node. There are two possibilities at the
         * moment: either 16-bit signed immediate value or a thing that can give 32-bit value.
         * The immediate expression is used with any mem access opcode directly, the 32-bit thing
         * requires additional code generation(thus it is a MACRO command) to reference memory that
         * far.
         *
         * Currently 32-bit expressions with mem reference are disallowed.
         */
        switch (*String_At (frmt, 0))
        {
        case SIGNED_OFFSET_16_BIT:
        {
            if (A_LiteralIsInteger (opd) &&
                    A_LiteralInRange (opd->u.mem.offset, INT16_MIN, INT16_MAX))
            {
                intmax_t offset = opd->u.mem.offset->u.ival;
                // FIXME do not use "b" literal
                IR_mipsOpd base = TrOpd (err, cntx, opd->u.mem.base, String_New ("b"), pos);
                iropd = IR_MipsOpdMem (offset, base);
            }
            else
            {
                *err = String_New (
                           "Signed offset must be a 16-bit value in range from -32,768 to 32,767");
            }

            break;
        }
        case MA_GENERAL_EXPRESSION_32_BIT:
        {
            *err = String_New ("32-bit expressions are temporary disallowed");
            break;
        }
        default:
        {
            assert (0);
            break;
        }
        }
    }
    else if (String_Size (frmt) == 1)
    {
        char c = *String_At (frmt, 0);
        switch (c)
        {
        case TARGET_REGISTER_5_BIT:
        case DESTINATION_REGISTER_5_BIT:
        case BASE_REGISTER_5_BIT:
        case SOURCE_REGISTER_5_BIT:
        case SAME_REGISTER_SOURCE_AND_TARGET_5_BIT:
        case SAME_REGISTER_SOURCE_AND_DESTINATION_5_BIT:
        case SAME_REGISTER_TARGET_AND_DESTINATION_5_BIT:
        {
            switch (opd->kind)
            {
            case A_asmOpRepKind:
            {
                // not implemented
                assert (0);
                break;
            }
            case A_asmOpVarKind:
            {
                /*
                 * Normally variable processing changes the sema context and because it is possible
                 * to evaluate the same variable several times during opc and opd matching we need
                 * to run it through this validation method to make sure it is possible to translate
                 * the var and change the semantic context once.
                 *
                 * TODO i have a dream...;) that all IR translation will be side-effect free
                 */
                if (Sema_ValidateVar (cntx->context, opd->u.var).ty != Ty_Invalid())
                {
                    Temp_temp temp = Temp_NewTemp();
                    Sema_Exp sexp = Sema_TransVar (cntx->context, opd->u.var, TRUE);
                    T_stm stm = T_Move (Tr_UnEx (sexp.exp), T_Temp (temp));
                    IR_mipsClosure cls = NULL;
                    LIST_PUSH (cls, IR_MipsClosureHeStm (stm, TRUE));
                    iropd = IR_MipsOpdRepExp (temp, IS_DST (c, pos), cls);
                }
                else
                {
                    *err = String_New ("Expected valid variable expression");
                }
                break;
            }
            case A_asmOpLitKind:
            {
                *err = String_New ("Literal expressions are not allowed here");
                break;
            }
            case A_asmOpRegKind:
            {
                Temp_temp temp = M_RegGet (regs_all, opd->u.reg->u.name);
                iropd = IR_MipsOpdRepExp (temp, IS_DST (c, pos), NULL);
                break;
            }
            case A_asmOpTmpKind:
            {
                Temp_temp temp = (Temp_temp)TAB_Look (cntx->meta_regs, opd->u.tmp);
                if (!temp)
                {
                    temp = Temp_NewTemp();
                    TAB_Enter (cntx->meta_regs, opd->u.tmp, temp);
                }
                iropd = IR_MipsOpdRepExp (temp, IS_DST (c, pos), NULL);
                break;
            }
            case A_asmOpLabKind:
            {
                *err = String_New ("Label are not allowed here");
                break;
            }
            case A_asmOpMemKind:
            {
                *err = String_New ("Mem expressions are not allowed here");
                break;
            }
            }

            break;
        }
        case ZERO_REGISTER:
        {
            switch (opd->kind)
            {
            case A_asmOpRegKind:
            {
                Temp_temp temp = M_RegGet (regs_all, opd->u.reg->u.name);
                if (temp == zero)
                {
                    iropd = IR_MipsOpdRepExp (temp, FALSE, NULL);
                }
                else
                {
                    *err = String_New ("Expected $zero register operand");
                }

                break;
            }
            default:
            {
                *err = String_New ("Expected $zero register");
                break;
            }
            }

            break;
        }
        case SHIFT_AMOUNT_5_BIT:
        {
            switch (opd->kind)
            {
            case A_asmOpLitKind:
            {
                if (A_LiteralIsInteger (opd->u.lit) &&
                        A_LiteralInRange (opd->u.lit , UINTMAX_C (0) , UINT5_MAX))
                {
                    iropd = IR_MipsOpdImmUInt (opd->u.lit->u.uval);
                }
                else
                {
                    *err = String_New ("Shift amount must be a 5-bit value in range from 0 to 31");
                }

                break;
            }
            default:
            {
                *err = String_New ("Shift amount must be a 5-bit value in range from 0 to 31");
                break;
            }
            }

            break;
        }
        /* case UPPER_16_BITS_OF_ADDRESS_16_BIT: */
        case UNSIGNED_IMMEDIATE_16_BIT:
        {
            switch (opd->kind)
            {
            case A_asmOpLitKind:
            {
                if (A_LiteralIsInteger (opd->u.lit) &&
                        A_LiteralInRange (opd->u.lit , UINTMAX_C (0), UINT16_MAX))
                {
                    iropd = IR_MipsOpdImmUInt (opd->u.lit->u.uval);
                }
                else
                {
                    *err = String_New ("Expected 16-bit imm value in range from 0 to 65,535");
                }

                break;
            }
            default:
            {
                *err = String_New ("Expected 16-bit imm value in range from 0 to 65,535");
                break;
            }
            }

            break;
        }
        case SIGNED_IMMEDIATE_16_BIT:
        {
            switch (opd->kind)
            {
            case A_asmOpLitKind:
            {
                if (A_LiteralIsInteger (opd->u.lit) &&
                        A_LiteralInRange (opd->u.lit , INT16_MIN, INT16_MAX))
                {
                    iropd = IR_MipsOpdImmInt (opd->u.lit->u.ival);
                }
                else
                {
                    *err = String_New ("Expected 16-bit imm value in range from -32,768 to 32,767");
                }

                break;
            }
            default:
            {
                *err = String_New ("Expected 16-bit imm value in range from -32,768 to 32,767");
                break;
            }
            }

            break;
        }
        case SYSCALL_FUNCTION_CODE_20_BIT:
        {
            switch (opd->kind)
            {
            case A_asmOpLitKind:
            {
                if (A_LiteralIsInteger (opd->u.lit) &&
                        A_LiteralInRange (opd->u.lit , UINTMAX_C (0), UINT20_MAX))
                {
                    iropd = IR_MipsOpdImmUInt (opd->u.lit->u.uval);
                }
                else
                {
                    *err = String_New ("Syscall function code must be a 20-bit value in range\
   from 0 to 1,048,575");
                }

                break;
            }
            default:
            {
                *err = String_New ("Syscall function code must be a 20-bit value in range\
   from 0 to 1,048,575");
                break;
            }
            }

            break;
        }
        // JUMP
        case TARGET_ADDRESS_26_BIT:
        {
            switch (opd->kind)
            {
            case A_asmOpRepKind:
            {
                // not implemented
                assert (0);
                break;
            }
            case A_asmOpVarKind:
            {
                if (opd->u.var->kind == A_simpleVar)
                {
                    Temp_label label = (Temp_label)TAB_Look (cntx->norm_labs, opd->u.var->u.simple);
                    if (label)
                    {
                        iropd = IR_MipsOpdRepJmp (label);
                    }
                    else
                    {
                        *err = String_New ("Expected a valid label");
                    }
                }
                else
                {
                    *err = String_New ("Variable expressions are not allowed here");
                }

                break;
            }
            case A_asmOpLitKind:
            {
                if (A_LiteralIsInteger (opd->u.lit) &&
                        A_LiteralInRange (opd->u.lit, INT26_MIN, INT26_MAX))
                {
                    iropd = IR_MipsOpdImmInt (opd->u.lit->u.ival);
                }
                else
                {
                    *err = String_New ("Target address must be a 26-bit value in range\
    from -33,554,432 to 33,554,431");
                }

                break;
            }
            case A_asmOpRegKind:
            {
                *err = String_New ("Register operand is not allowed here");
                break;
            }
            case A_asmOpTmpKind:
            {
                *err = String_New ("Register operand is not allowed here");
                break;
            }
            case A_asmOpLabKind:
            {
                Temp_label label = (Temp_label)TAB_Look (cntx->meta_labs, opd->u.lab);
                if (label)
                {
                    iropd = IR_MipsOpdRepJmp (label);
                }
                else
                {
                    *err = String_New ("Expected a valid meta label");
                }

                break;
            }
            case A_asmOpMemKind:
            {
                *err = String_New ("Mem operand is not allowed here");
                break;
            }
            }

            break;
        }
        // BRANCH
        case PC_RELATIVE_BRANCH_TARGET_16_BIT:
        {
            switch (opd->kind)
            {
            case A_asmOpRepKind:
            {
                // not implemented
                assert (0);
                break;
            }
            case A_asmOpVarKind:
            {
                if (opd->u.var->kind == A_simpleVar)
                {
                    Temp_label label = (Temp_label)TAB_Look (cntx->norm_labs, opd->u.var->u.simple);
                    if (label)
                    {
                        iropd = IR_MipsOpdRepJmp (label);
                    }
                    else
                    {
                        *err = String_New ("Expected a valid label");
                    }
                }
                else
                {
                    *err = String_New ("Variable expressions are not allowed here");
                }

                break;
            }
            case A_asmOpLitKind:
            {
                if (A_LiteralIsInteger (opd->u.lit) &&
                        A_LiteralInRange (opd->u.lit, INT16_MIN, INT16_MAX))
                {
                    iropd = IR_MipsOpdImmInt (opd->u.lit->u.ival);
                }
                else
                {
                    *err = String_New ("Target address must be a 16-bit value in range \
    from -2,147,483,648 to 2,147,483,647");
                }

                break;
            }
            case A_asmOpRegKind:
            {
                *err = String_New ("Register operand is not allowed here");
                break;
            }
            case A_asmOpTmpKind:
            {
                *err = String_New ("Register operand is not allowed here");
                break;
            }
            case A_asmOpLabKind:
            {
                Temp_label label = (Temp_label)TAB_Look (cntx->meta_labs, opd->u.lab);
                if (label)
                {
                    iropd = IR_MipsOpdRepJmp (label);
                }
                else
                {
                    *err = String_New ("Expected a valid meta label");
                }

                break;
            }
            case A_asmOpMemKind:
            {
                *err = String_New ("Mem operand is not allowed here");
                break;
            }
            }

            break;
        }
        case MA_GENERAL_EXPRESSION_32_BIT:
        {
            switch (opd->kind)
            {
            case A_asmOpRepKind:
            {
                // not implemented
                assert (0);
                break;
            }
            case A_asmOpVarKind:
            {
                if (opd->u.var->kind == A_simpleVar)
                {
                    Temp_label label = (Temp_label)TAB_Look (cntx->norm_labs, opd->u.var->u.simple);
                    if (label)
                    {
                        iropd = IR_MipsOpdLab (label);
                    }
                    else
                    {
                        *err = String_New ("Expected a valid label");
                    }
                }
                else
                {
                    //TODO implement var expressions
                    *err = String_New ("Variable expressions are not allowed here");
                }

                break;
            }
            case A_asmOpLitKind:
            {
                if (A_LiteralIsInteger (opd->u.lit) &&
                        A_LiteralInRange (opd->u.lit, INT32_MIN, INT32_MAX))
                {
                    iropd = IR_MipsOpdImmInt (opd->u.lit->u.ival);
                }
                else
                {
                    *err = String_New ("Target address must be a 16-bit value in range \
    from -2,147,483,648 to 2,147,483,647");
                }

                break;
            }
            case A_asmOpRegKind:
            {
                *err = String_New ("Register operand is not allowed here");
                break;
            }
            case A_asmOpTmpKind:
            {
                *err = String_New ("Register operand is not allowed here");
                break;
            }
            case A_asmOpLabKind:
            {
                Temp_label label = (Temp_label)TAB_Look (cntx->meta_labs, opd->u.lab);
                if (label)
                {
                    iropd = IR_MipsOpdLab (label);
                }
                else
                {
                    *err = String_New ("Expected a valid meta label");
                }

                break;
            }
            case A_asmOpMemKind:
            {
                *err = String_New ("Mem operand is not allowed here");
                break;
            }
            }

            break;
        }
        case MA_IMMEDIATE_32_BIT:
        {
            switch (opd->kind)
            {
            case A_asmOpLitKind:
            {
                if (A_LiteralIsInteger (opd->u.lit) &&
                        A_LiteralInRange (opd->u.lit , UINTMAX_C (0), UINT32_MAX))
                {
                    iropd = IR_MipsOpdImmInt (opd->u.lit->u.uval);
                }
                else
                {
                    *err = String_New ("Expected 32-bit imm value");
                }

                break;
            }
            default:
            {
                *err = String_New ("Expected 32-bit imm value");
                break;
            }
            }

            break;
        }
        // TODO this must go into closure
        case MA_LITERAL_ADDRESS:
        {
            switch (opd->kind)
            {
            case A_asmOpLitKind:
            {
                switch (opd->u.lit->kind)
                {
                case A_literalString:
                {
                    Temp_label label = Temp_NewLabel();
                    Program_AddFragment (
                        cntx->module,
                        F_StringFrag (label, opd->u.lit->u.sval, F_lps));
                    iropd = IR_MipsOpdLab (label);
                    break;
                }
                default:
                {
                    assert (0);
                    break;
                }
                }

                break;
            }
            default:
            {
                *err = String_New ("Expected litaral value");
                break;
            }
            }

            break;
        }
        }
    }

    return iropd;
}

/*
 * The process of opcode selection is data driven, there is opcodes table that defines available
 * instructions and format line that we try to match against. There can be multiple format lines
 * for a opcode, if none of them match we consider opcode AST node as invalid.
 */
static IR_mipsStm TrStmOpc (String * err, Sema_mipsContext cntx, A_asmStmInst opc)
{
    IR_mipsStm irstm = NULL;

    /*
     * First we gonna collect all matched opcodes, there can be more than one signature.
     */
    struct Vector_t candidates = Vector (const struct M_opCode_t *);

    for (size_t i = 0; i < mips_opcodes_num; ++i)
    {
        if (String_Equal (&mips_opcodes[i].name, opc->opcode))
        {
            Vector_PushBack (&candidates, &mips_opcodes[i]);
        }
    }

    const struct M_opCode_t * opcode = NULL;

    // this var stores rejection explanations for the opcodes candidates
    struct Vector_t rejections = Vector (String);

    VECTOR_FOREACH (const struct M_opCode_t *, copc, &candidates)
    {
        opcode = *copc;

        Vector format = String_Split (&opcode->format, ',');

        IR_mipsOpd iropd = NULL;
        IR_mipsOpdList iropdl = NULL;

        A_asmOpList opdl = opc->opList;
        VECTOR_FOREACH (struct String_t, frmt, format)
        {
            if (!opdl)
            {
                Vector_PushBack (&rejections, String_New ("Not enough operands"));
                opcode = NULL;
                break;
            }

            String error = NULL;
            iropd = TrOpd (&error, cntx, opdl->head, frmt, __i_frmt);

            if (!iropd)
            {
                Vector_PushBack (&rejections, error);
                opcode = NULL;
                break;
            }

            LIST_PUSH (iropdl, iropd);

            opdl = LIST_NEXT (opdl);
        }

        /*
         * We found almost correct opcode but there are to much operands
         */
        if (opcode && opdl)
        {
            Vector_PushBack (&rejections, String_New ("Too many operands"));
            opcode = NULL;
            break;
        }

        /*
         * If we got here then we have a validly parsed IR for the AST opcode
         */
        if (opcode)
        {
            irstm = IR_MipsStmOpc (opcode, iropdl);
            break;
        }
    }

    if (!irstm)
    {
        if (Vector_Size (&rejections))
        {
            *err = String_New (
                       "Could not find proper opcode signature, here are some candidates:");

            for (size_t i = 0; i < Vector_Size (&rejections); ++i)
            {
                struct M_opCode_t * candidate = * (M_opCode *)Vector_At (&candidates, i);
                String rejection = * (String *)Vector_At (&rejections, i);

                // calculate additional string size we need
                size_t reserve = 128
                                 + String_Size (*err)
                                 + String_Size (&candidate->name)
                                 + String_Size (&candidate->format)
                                 + String_Size (rejection);

                String_Reserve (*err, reserve);
                String_AppendF (*err, "\n'%s %s' is rejected because '%s'",
                                candidate->name.data,
                                candidate->format.data,
                                rejection->data);
            }
        }
        /*
         * If there is no rejections then were no match candidates thus opcode is unknown
         */
        else
        {
            *err = String_New ("Unknown opcode");
        }

        return NULL;
    }

    return irstm;
}

static IR_mipsStm TrStmLab (String * err, Sema_mipsContext cntx, A_asmStmLab lab)
{
    *err = NULL;

    TAB_table table = lab->meta ? cntx->meta_labs : cntx->norm_labs;
    Temp_label label = (Temp_label)TAB_Look (table, lab->sym);

    return IR_MipsStmLab (label);
}

static IR_mipsStm TrStm (String * err, Sema_mipsContext cntx, A_asmStm stm)
{
    switch (stm->kind)
    {
    case A_asmStmInstKind:
    {
        return TrStmOpc (err, cntx, &stm->u.inst);
    }
    case A_asmStmLabKind:
    {
        return TrStmLab (err, cntx, &stm->u.lab);
    }
    default:
    {
        assert (0);
    }
    }
}

IR_mipsStmList SemantMIPS_Translate (Sema_Context cntx, struct A_asmDec_t * dec)
{
    assert (cntx);
    assert (dec);

    struct Sema_mipsContext_t context;
    context.dec = dec;
    context.context = cntx;
    context.module = cntx->module;
    Vector_Init (&context.labels, Temp_label);
    context.errors = 0;
    context.meta_regs = TAB_Empty();
    context.meta_labs = TAB_Empty();
    context.norm_labs = TAB_Empty();

    if (!PreProcess (&context, dec))
    {
        return NULL;
    }

    IR_mipsStmList result = NULL;
    LIST_FOREACH (stm, dec->code)
    {
        String err = NULL;
        IR_mipsStm irstm = TrStm (&err, &context, stm);

        if (err)
        {
            ERROR (&stm->loc, 3100, "%s", err->data);
            continue;
        }

        LIST_PUSH (result, irstm);
    }

    return result;
}
