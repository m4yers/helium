#ifndef ABSYN_H_MZGLRGWC
#define ABSYN_H_MZGLRGWC

#include <stdio.h>

#include "ext/util.h"
#include "ext/bool.h"

#include "symbol.h"

/* Type Definitions */

typedef int A_pos;

typedef struct A_loc_
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
  //NOTE token MUST NOT be used outside lexer and parser
  const char * token;
} * A_loc;

typedef struct A_var_ * A_var;
typedef struct A_exp_ * A_exp;
typedef struct A_dec_ * A_dec;
typedef struct A_ty_ * A_ty;
typedef struct A_literal_ * A_literal;

typedef struct A_spec_ * A_spec;
typedef struct A_specList_ * A_specList;

typedef struct A_decList_ * A_decList;
typedef struct A_expList_ * A_expList;
typedef struct A_field_ * A_field;
typedef struct A_fieldList_ * A_fieldList;
typedef struct A_typeDef_ * A_typeDef;
typedef struct A_efield_ * A_efield;
typedef struct A_efieldList_ * A_efieldList;

typedef struct A_scope_ * A_scope;

typedef enum
{
    A_plusOp, A_minusOp, A_timesOp, A_divideOp,
    A_eqOp, A_neqOp, A_ltOp, A_leOp, A_gtOp, A_geOp
} A_oper;

struct A_var_
{
    enum
    {
        A_simpleVar,
        A_fieldVar,
        A_subscriptVar
    } kind;
    A_pos pos;
    union
    {
        S_symbol simple;
        struct
        {
            A_var var;
            S_symbol sym;
        } field;
        struct
        {
            A_var var;
            A_exp exp;
        } subscript;
    } u;
};

struct A_exp_
{
    enum
    {
        A_varExp, A_nilExp, A_intExp, A_stringExp, A_callExp,
        A_opExp, A_recordExp, A_seqExp, A_assignExp, A_ifExp,
        A_whileExp, A_forExp, A_breakExp, A_arrayExp
    } kind;
    A_pos pos;
    union
    {
        A_var var;

        /* nil; - needs only the pos */

        int intt;

        string stringg;

        struct
        {
            S_symbol func;
            A_expList args;
        } call;

        struct
        {
            A_oper oper;
            A_exp left;
            A_exp right;
        } op;

        struct
        {
            S_symbol name;
            A_efieldList fields;
        } record;

        A_expList seq;

        struct
        {
            A_var var;
            A_exp exp;
        } assign;

        struct
        {
            A_exp test;
            A_scope tr, fl;
        } iff; /* elsee is optional */

        struct
        {
            A_exp test;
            A_scope body;
        } whilee;

        struct
        {
            S_symbol var;
            A_exp lo, hi;
            A_scope body;
            bool escape;
        } forr;

        /* breakk; - need only the pos */

        A_expList array;
    } u;
};

struct A_dec_

{
    enum
    {
        A_typeDec,
        A_functionDec,
        A_varDec,
    } kind;

    union
    {
        struct
        {
            A_pos pos;
            S_symbol name;
            A_fieldList params;
            A_ty type;
            A_scope scope;
        } function;

        struct
        {
            S_symbol var;
            A_ty type;
            A_exp init;
            bool escape;
        } var;

        struct
        {
            S_symbol name;
            A_ty type;
        } type;
    } u;

    A_pos pos;
};

struct A_ty_
{
    enum
    {
        A_nameTy,
        A_arrayTy,
        A_recordTy,

    } kind;

    union
    {
        S_symbol name;
        A_fieldList record;
        A_expList array;
    } u;

    A_specList specs;

    A_pos pos;
};

struct A_literal_
{
    enum
    {
        A_literalBool,
        A_literalInt,
        A_literalFloat,
        A_literalString
    } kind;

    union
    {
        bool boolean;
        int integer;
        double fp;
        const char * string;
    } u;

    int pos;
};

struct A_spec_
{
    enum
    {
        A_specType,
        A_specLiteral
    } kind;

    union
    {
        A_ty type;
        A_literal literal;
    } u;

    int pos;
};

struct A_specList_
{
    A_spec head;
    A_specList tail;
};

/* Linked lists and nodes of lists */

struct A_field_
{
    struct A_loc_ loc;
    S_symbol name;
    A_ty type;
    bool escape;
};
struct A_fieldList_
{
    A_field head;
    A_fieldList tail;
};
struct A_expList_
{
    A_exp head;
    A_expList tail;
};

struct A_decList_
{
    A_dec head;
    A_decList tail;
};
struct A_efield_
{
    S_symbol name;
    A_exp exp;
};
struct A_efieldList_
{
    A_efield head;
    A_efieldList tail;
};

typedef struct A_stm_
{
    enum  { A_stmDec, A_stmExp } kind;
    union
    {
        A_dec dec;
        A_exp exp;
    } u;
} * A_stm;

typedef struct A_stmList_
{
    A_stm head;
    struct A_stmList_ * tail;
} * A_stmList;

A_stm A_StmExp (A_exp exp);
A_stm A_StmDec (A_dec exp);
A_stmList A_StmList (A_stm head, A_stmList tail);

struct A_scope_
{
    A_stmList list;
};

A_scope A_Scope (A_stmList list);

A_literal A_LiteralBool (A_pos pos, bool value);
A_literal A_LiteralInt (A_pos pos, int value);
A_literal A_LiteralFloat (A_pos pos, double value);
A_literal A_LiteralString (A_pos pos, const char * value);

A_spec A_SpecType (A_pos pos, A_ty type);
A_spec A_SpecLiteral (A_pos pos, A_literal literal);
A_specList A_SpecList (A_spec head, A_specList tail);

A_var A_SimpleVar (A_pos pos, S_symbol sym);
A_var A_FieldVar (A_pos pos, A_var var, S_symbol sym);
A_var A_SubscriptVar (A_pos pos, A_var var, A_exp exp);

A_exp A_VarExp (A_pos pos, A_var var);
A_exp A_NilExp (A_pos pos);
A_exp A_IntExp (A_pos pos, int i);
A_exp A_StringExp (A_pos pos, string s);
A_exp A_CallExp (A_pos pos, S_symbol func, A_expList args);
A_exp A_OpExp (A_pos pos, A_oper oper, A_exp left, A_exp right);
A_exp A_RecordExp (A_pos pos, S_symbol name, A_efieldList fields);
A_exp A_SeqExp (A_pos pos, A_expList seq);
A_exp A_AssignExp (A_pos pos, A_var var, A_exp exp);
A_exp A_IfExp (A_pos pos, A_exp test, A_scope tr, A_scope fl);
A_exp A_WhileExp (A_pos pos, A_exp test, A_scope body);
A_exp A_ForExp (A_pos pos, S_symbol var, A_exp lo, A_exp hi, A_scope body);
A_exp A_BreakExp (A_pos pos);
A_exp A_ArrayExp (A_pos pos, A_expList list);
A_expList A_ExpList (A_exp head, A_expList tail);

A_dec A_FunctionDec (A_pos pos, S_symbol name, A_fieldList params, A_ty type, A_scope scope);
A_dec A_VarDec (A_pos pos, S_symbol var, A_ty type, A_exp init);
A_dec A_TypeDec (S_symbol name, A_ty type);
A_decList A_DecList (A_dec head, A_decList tail);

A_ty A_NameTy (A_pos pos, S_symbol name, A_specList specs);
A_ty A_ArrayTy (A_pos pos, A_expList list);
A_ty A_RecordTy (A_pos pos, A_fieldList record);

A_field A_Field (A_loc loc, S_symbol name, A_ty type);
A_fieldList A_FieldList (A_field head, A_fieldList tail);

A_efield A_Efield (S_symbol name, A_exp exp);
A_efieldList A_EfieldList (A_efield head, A_efieldList tail);

void AST_Print (FILE * out, A_decList list, int d);

#endif /* end of include guard: ABSYN_H_MZGLRGWC */
