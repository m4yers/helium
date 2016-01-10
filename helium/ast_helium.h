#ifndef ABSYN_H_MZGLRGWC
#define ABSYN_H_MZGLRGWC

#include <stdio.h>

#include "ext/util.h"
#include "ext/list.h"
#include "ext/bool.h"

#include "ast.h"
#include "ast_asm.h"

#include "symbol.h"

typedef struct A_var_t * A_var;
typedef struct A_exp_t * A_exp;
typedef struct A_dec_t * A_dec;
typedef struct A_ty_t * A_ty;
typedef struct A_literal_t * A_literal;
typedef struct A_spec_t * A_spec;
typedef struct A_field_t * A_field;
typedef struct A_typeDef_t * A_typeDef;
typedef struct A_efield_t * A_efield;
typedef struct A_stm_t * A_stm;
typedef struct A_scope_t * A_scope;

LIST_DEFINE (A_fieldList, A_field)
LIST_DEFINE (A_efieldList, A_efield)
LIST_DEFINE (A_expList, A_exp)
LIST_DEFINE (A_decList, A_dec)
LIST_DEFINE (A_specList, A_spec)
LIST_DEFINE (A_stmList, A_stm)

/****************
 *  Node types  *
 ****************/

typedef enum
{
    /** Var */
    A_simpleVar, A_fieldVar, A_subscriptVar,

    /** Dec */
    A_typeDec, A_functionDec, A_varDec, A_asmDec,

    A_asmExpOld, // <- remove this

    /** Exp */
    A_retExp, A_addressOf, A_valueAt, A_typeCastExp,
    A_varExp, A_nilExp, A_intExp, A_stringExp, A_callExp, A_macroCallExp,
    A_opExp, A_recordExp, A_seqExp, A_assignExp, A_ifExp,
    A_whileExp, A_forExp, A_breakExp, A_arrayExp,

    /** Types */
    A_nameTy, A_pointerTy, A_arrayTy, A_recordTy,
} A_nodeKind;

/************
 *  Fields  *
 ************/

struct A_field_t
{
    struct A_loc_t loc;
    S_symbol name;
    A_ty type;
    bool escape;
};

A_field A_Field (A_loc loc, S_symbol name, A_ty type);
A_fieldList A_FieldList (A_field head, A_fieldList tail);

struct A_efield_t
{
    struct A_loc_t loc;
    S_symbol name;
    A_exp exp;
};

A_efield A_Efield (A_loc loc, S_symbol name, A_exp exp);
A_efieldList A_EfieldList (A_efield head, A_efieldList tail);

/*********
 *  LHS  *
 *********/

struct A_varField_t
{
    A_var var;
    S_symbol sym;
    int jumps;
};

struct A_varSubscript_t
{
    A_var var;
    A_exp exp;
};

struct A_var_t
{
    struct A_loc_t loc;

    A_nodeKind kind;

    union
    {
        S_symbol simple;
        struct A_varField_t field;
        struct A_varSubscript_t subscript;
    } u;
};

A_var A_SimpleVar (A_loc loc, S_symbol sym);
A_var A_FieldVar (A_loc loc, A_var var, S_symbol sym, int jumps);
A_var A_SubscriptVar (A_loc loc, A_var var, A_exp exp);

/*****************
 *  Expressions  *
 *****************/

typedef enum
{
    A_plusOp, A_minusOp, A_timesOp, A_divideOp,
    A_eqOp, A_neqOp, A_ltOp, A_leOp, A_gtOp, A_geOp
} A_oper;

struct A_opExp_t
{
    A_oper oper;
    A_exp left;
    A_exp right;
};

struct A_callExp_t
{
    S_symbol func;
    A_expList args;
};

// TODO rename to left and right
struct A_assignExp_t
{
    A_var var;
    A_exp exp;
};

struct A_ifExp_t
{
    A_exp test;
    A_scope tr, fl;
};

struct A_whileExp_t
{
    A_exp test;
    A_scope body;
};

struct A_forExp_t
{
    S_symbol var;
    A_exp lo, hi;
    A_scope body;
    bool escape;
};

struct A_asmExpOld_t
{
    const char * code;
    const char * data;
    U_stringList dst;
    U_stringList src;
};

struct A_macroExp_t
{
    S_symbol name;
    A_expList args;
};

struct A_recordExp_t
{
    S_symbol name;
    A_efieldList fields;
};

struct A_typeCastExp_t
{
    A_ty type;
    A_exp exp;
};

struct A_exp_t
{
    struct A_loc_t loc;

    A_nodeKind kind;

    union
    {
        /* break - need only the pos */
        /* nil; - needs only the pos */
        struct A_typeCastExp_t typeCast;
        A_var addressOf;
        A_exp valueAt;
        A_expList seq;
        A_var var;
        A_exp ret;
        int intt;
        const char * stringg;
        struct A_asmExpOld_t  asmOld;
        struct A_callExp_t call;
        struct A_macroExp_t macro;
        struct A_opExp_t op;
        struct A_recordExp_t record;
        struct A_assignExp_t assign;
        struct A_ifExp_t iff;
        struct A_whileExp_t whilee;
        struct A_forExp_t forr;
        A_expList array;
    } u;
};

A_exp A_TypeCastExp(A_loc loc, A_ty type, A_exp exp);
A_exp A_AddressOfExp (A_loc loc, A_var var);
A_exp A_ValueAtExp (A_loc loc, A_exp exp);

// TODO parse it for real
// TODO data must be and exp
A_exp A_AsmExpOld (A_loc loc, const char * code, U_stringList dst, U_stringList src, const char * data);
A_exp A_AsmExp (A_loc loc, U_stringList options, A_asmStmList code, A_expList out, A_expList in);

A_exp A_VarExp (A_loc loc, A_var var);
A_exp A_NilExp (A_loc loc);
A_exp A_IntExp (A_loc loc, int i);
A_exp A_StringExp (A_loc loc, const char * s);
A_exp A_CallExp (A_loc loc, S_symbol func, A_expList args);
A_exp A_MacroCallExp (A_loc loc, S_symbol name, A_expList args);
A_exp A_OpExp (A_loc loc, A_oper oper, A_exp left, A_exp right);
A_exp A_RecordExp (A_loc loc, S_symbol name, A_efieldList fields);
A_exp A_SeqExp (A_loc loc, A_expList seq);
A_exp A_AssignExp (A_loc loc, A_var var, A_exp exp);
A_exp A_IfExp (A_loc loc, A_exp test, A_scope tr, A_scope fl);
A_exp A_WhileExp (A_loc loc, A_exp test, A_scope body);
A_exp A_ForExp (A_loc loc, S_symbol var, A_exp lo, A_exp hi, A_scope body);
A_exp A_BreakExp (A_loc loc);
A_exp A_RetExp (A_loc loc, A_exp exp);
A_exp A_ArrayExp (A_loc loc, A_expList list);
A_expList A_ExpList (A_exp head, A_expList tail);

/******************
 *  Declarations  *
 ******************/

struct A_AsmDec_t
{
    A_asmStmList code;
    U_stringList options;
    A_expList out;
    A_expList in;
};

struct A_decFn_t
{
    S_symbol name;
    A_fieldList params;
    A_ty type;
    A_scope scope;
};

struct A_decVar_t
{
    S_symbol var;
    A_ty type;
    A_exp init;
    bool escape;
};

struct A_decType_t
{
    S_symbol name;
    A_ty type;
};

struct A_dec_t
{
    struct A_loc_t loc;

    A_nodeKind kind;

    union
    {
        struct A_AsmDec_t assembly;
        struct A_decFn_t function;
        struct A_decVar_t var;
        struct A_decType_t type;
    } u;
};

A_dec A_FunctionDec (A_loc loc, S_symbol name, A_fieldList params, A_ty type, A_scope scope);
A_dec A_VarDec (A_loc loc, S_symbol var, A_ty type, A_exp init);
A_dec A_TypeDec (A_loc loc, S_symbol name, A_ty type);
A_dec A_AsmDec (A_loc loc, U_stringList options, A_asmStmList code, A_expList out, A_expList in);
A_decList A_DecList (A_dec head, A_decList tail);

/***********
 *  Types  *
 ***********/

struct A_arrayTy_t
{
    A_ty type;
    A_exp size;
};

struct A_ty_t
{
    struct A_loc_t loc;

    A_nodeKind kind;

    union
    {
        S_symbol name;
        A_ty pointer;
        A_fieldList record;
        struct A_arrayTy_t array;
    } u;

    A_specList specs;
};

A_ty A_NameTy (A_loc loc, S_symbol name, A_specList specs);
A_ty A_PointerTy (A_loc loc, A_ty type);
A_ty A_ArrayTy (A_loc loc, A_ty type, A_exp size);
A_ty A_RecordTy (A_loc loc, A_fieldList record);

/**************
 *  Literals  *
 **************/

struct A_literal_t
{
    struct A_loc_t loc;

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

A_literal A_LiteralBool (A_loc loc, bool value);
A_literal A_LiteralInt (A_loc loc, int value);
A_literal A_LiteralFloat (A_loc loc, double value);
A_literal A_LiteralString (A_loc loc, const char * value);

/****************
 *  Statements  *
 ****************/

struct A_stm_t
{
    enum  { A_stmDec, A_stmExp } kind;
    union
    {
        A_dec dec;
        A_exp exp;
    } u;
};

A_stm A_StmExp (A_exp exp);
A_stm A_StmDec (A_dec exp);
A_stmList A_StmList (A_stm head, A_stmList tail);

/***********
 *  Scope  *
 ***********/

struct A_scope_t
{
    A_stmList list;
};

A_scope A_Scope (A_stmList list);

/***********
 *  Specs  *
 ***********/

struct A_spec_t
{
    struct A_loc_t loc;

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

A_spec A_SpecType (A_loc loc, A_ty type);
A_spec A_SpecLiteral (A_loc loc, A_literal literal);
A_specList A_SpecList (A_spec head, A_specList tail);

/*************
 *  Printer  *
 *************/

void AST_Print (FILE * out, A_decList list, int d);
void AST_PrintIndent (FILE * out, int d);
void AST_PrintScope (FILE * out, A_scope scope, int d);
void AST_PrintLiteral (FILE * out, A_literal literal, int d);
void AST_PrintSpecs (FILE * out, A_specList specs, int d);
void AST_PrintExp (FILE * out, A_exp v, int d);
void AST_PrintOp (FILE * out, A_oper d);
void AST_PrintVar (FILE * out, A_var v, int d);
void AST_PrintDec (FILE * out, A_dec v, int d);
void AST_PrintType (FILE * out, A_ty v, int d);
void AST_PrintField (FILE * out, A_field v, int d);
void AST_PrintFieldList (FILE * out, A_fieldList v, int d);
void AST_PrintExpList (FILE * out, A_expList v, int d);
void AST_PrintExpField (FILE * out, A_efield v, int d);
void AST_PrintExpFieldList (FILE * out, A_efieldList v, int d);


#endif /* end of include guard: ABSYN_H_MZGLRGWC */
