#ifndef TREE_H_P4J0KEXI
#define TREE_H_P4J0KEXI

#include "modules/helium/ast.h"
#include "modules/mips/ast.h"
#include "modules/mips/ir.h"

#include "core/temp.h"

typedef struct T_stm_t * T_stm;
typedef struct T_exp_t * T_exp;

LIST_DEFINE(T_expList, T_exp)
LIST_DEFINE(T_stmList, T_stm)

typedef enum
{
    T_plus, T_minus, T_mul, T_div,
    T_and, T_or, T_lshift, T_rshift, T_arshift, T_xor
} T_binOp ;

typedef enum
{
    T_eq, T_ne, T_lt, T_gt, T_le, T_ge,
    T_ult, T_ule, T_ugt, T_uge
} T_relOp;

struct T_stm_t
{
    enum
    {
        T_COMMENT, T_ASM, T_RET, T_EXIT,
        T_SEQ, T_LABEL, T_JUMP, T_RJUMP, T_CJUMP, T_MOVE,
        T_EXP
    } kind;
    union
    {
        const char * COMMENT;
        struct
        {
            IR_mipsStmList stms;
        } ASSEMBLY;
        struct
        {
            T_stm left, right;
        } SEQ;
        Temp_label LABEL;
        struct
        {
            T_exp exp;
            Temp_labelList jumps;
        } JUMP;
        struct
        {
            T_exp src;
        } RJUMP;
        struct
        {
            T_relOp op;
            T_exp left, right;
            Temp_label true, false;
        } CJUMP;
        struct
        {
            T_exp dst, src;
        } MOVE;
        T_exp EXP;
        T_exp RET;
        T_exp EXIT;
    } u;
};

struct T_exp_t
{
    enum
    {
        T_BINOP, T_MEM, T_TEMP, T_ESEQ, T_NAME,
        T_CONST, T_CALL
    } kind;
    union
    {
        struct
        {
            T_binOp op;
            T_exp left, right;
        } BINOP;
        T_exp MEM;
        Temp_temp TEMP;
        struct
        {
            T_stm stm;
            T_exp exp;
        } ESEQ;
        Temp_label NAME;
        int CONST;
        struct
        {
            T_exp fun;
            T_expList args;
        } CALL;
    } u;
};

T_expList T_ExpList (T_exp head, T_expList tail);
int T_ExpListLen (T_expList list);
T_stmList T_StmList (T_stm head, T_stmList tail);

T_stm T_Asm (IR_mipsStmList stms);
T_stm T_AsmOld (const char * code, T_exp data, Temp_tempList dst, Temp_tempList src);
T_stm T_Seq (T_stm left, T_stm right);
T_stm T_Comment (const char * comment);
T_stm T_Label (Temp_label);
T_stm T_Ret (T_exp exp);
T_stm T_Exit (T_exp exp);
T_stm T_Jump (T_exp exp, Temp_labelList labels);
T_stm T_Rjump (T_exp src);
T_stm T_Cjump (T_relOp op, T_exp left, T_exp right,
               Temp_label true, Temp_label false);
T_stm T_Move (T_exp dst, T_exp src);
T_stm T_Exp (T_exp);

T_exp T_Binop (T_binOp, T_exp, T_exp);
T_exp T_Mem (T_exp);
T_exp T_Temp (Temp_temp);
T_exp T_Eseq (T_stm, T_exp);
T_exp T_Name (Temp_label);
T_exp T_Const (int);
T_exp T_Call (T_exp, T_expList);

T_stm T_NoOp (void);
T_exp T_Nil (void);
T_exp T_Void (void);

T_relOp T_notRel (T_relOp); /* a op b    ==     not(a notRel(op) b)  */
T_relOp T_commute (T_relOp); /* a op b    ==    b commute(op) a       */

#endif /* end of include guard: TREE_H_P4J0KEXI */
