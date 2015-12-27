#include <assert.h>

#include "ext/util.h"
#include "ext/mem.h"
#include "ext/list.h"
#include "ext/bool.h"

#include "symbol.h"
#include "ast.h"

/************
 *  Fields  *
 ************/

A_field A_Field (A_loc loc, S_symbol name, A_ty type)
{
    A_field p = checked_malloc (sizeof (*p));
    p->loc = *loc;
    p->name = name;
    p->type = type;
    p->escape = TRUE;
    return p;
}

A_fieldList A_FieldList (A_field head, A_fieldList tail)
{
    A_fieldList p = checked_malloc (sizeof (*p));
    p->head = head;
    p->tail = tail;
    return p;
}

A_efield A_Efield (A_loc loc, S_symbol name, A_exp exp)
{
    A_efield p = checked_malloc (sizeof (*p));
    p->loc = *loc;
    p->name = name;
    p->exp = exp;
    return p;
}

A_efieldList A_EfieldList (A_efield head, A_efieldList tail)
{
    A_efieldList p = checked_malloc (sizeof (*p));
    p->head = head;
    p->tail = tail;
    return p;
}

/*********
 *  LHS  *
 *********/

A_var A_SimpleVar (A_loc loc, S_symbol sym)
{
    A_var p = checked_malloc (sizeof (*p));
    p->kind = A_simpleVar;
    p->loc = *loc;
    p->u.simple = sym;
    return p;
}

A_var A_FieldVar (A_loc loc, A_var var, S_symbol sym, int jumps)
{
    A_var p = checked_malloc (sizeof (*p));
    p->kind = A_fieldVar;
    p->loc = *loc;
    p->u.field.var = var;
    p->u.field.sym = sym;
    p->u.field.jumps = jumps;
    return p;
}

A_var A_SubscriptVar (A_loc loc, A_var var, A_exp exp)
{
    A_var p = checked_malloc (sizeof (*p));
    p->kind = A_subscriptVar;
    p->loc = *loc;
    p->u.subscript.var = var;
    p->u.subscript.exp = exp;
    return p;
}

/*****************
 *  Expressions  *
 *****************/

A_exp A_TypeCastExp(A_loc loc, A_ty type, A_exp exp)
{
    A_exp p = checked_malloc (sizeof (*p));
    p->kind = A_typeCastExp;
    p->loc = *loc;
    p->u.typeCast.type = type;
    p->u.typeCast.exp = exp;
    return p;
}

A_exp A_AddressOfExp (A_loc loc, A_var var)
{
    A_exp p = checked_malloc (sizeof (*p));
    p->kind = A_addressOf;
    p->loc = *loc;
    p->u.addressOf = var;
    return p;
}

A_exp A_ValueAtExp (A_loc loc, A_exp exp)
{
    A_exp p = checked_malloc (sizeof (*p));
    p->kind = A_valueAt;
    p->loc = *loc;
    p->u.valueAt = exp;
    return p;
}

A_exp A_AsmExp (A_loc loc, const char * code, U_stringList dst, U_stringList src, const char * data)
{
    A_exp p = checked_malloc (sizeof (*p));
    p->kind = A_asmExp;
    p->loc = *loc;
    p->u.assembly.code = code;
    p->u.assembly.data = data;
    p->u.assembly.dst = dst;
    p->u.assembly.src = src;
    return p;
}

A_exp A_VarExp (A_loc loc, A_var var)
{
    A_exp p = checked_malloc (sizeof (*p));
    p->kind = A_varExp;
    p->loc = *loc;
    p->u.var = var;
    return p;
}

A_exp A_NilExp (A_loc loc)
{
    A_exp p = checked_malloc (sizeof (*p));
    p->kind = A_nilExp;
    p->loc = *loc;
    return p;
}

A_exp A_IntExp (A_loc loc, int i)
{
    A_exp p = checked_malloc (sizeof (*p));
    p->kind = A_intExp;
    p->loc = *loc;
    p->u.intt = i;
    return p;
}

A_exp A_StringExp (A_loc loc, const char * s)
{
    A_exp p = checked_malloc (sizeof (*p));
    p->kind = A_stringExp;
    p->loc = *loc;
    p->u.stringg = s;
    return p;
}

A_exp A_CallExp (A_loc loc, S_symbol func, A_expList args)
{
    A_exp p = checked_malloc (sizeof (*p));
    p->kind = A_callExp;
    p->loc = *loc;
    p->u.call.func = func;
    p->u.call.args = args;
    return p;
}

A_exp A_MacroCallExp (A_loc loc, S_symbol name, A_expList args)
{
    A_exp p = checked_malloc (sizeof (*p));
    p->kind = A_macroCallExp;
    p->loc = *loc;
    p->u.macro.name = name;
    p->u.macro.args = args;
    return p;

}

A_exp A_OpExp (A_loc loc, A_oper oper, A_exp left, A_exp right)
{
    A_exp p = checked_malloc (sizeof (*p));
    p->kind = A_opExp;
    p->loc = *loc;
    p->u.op.oper = oper;
    p->u.op.left = left;
    p->u.op.right = right;
    return p;
}

A_exp A_RecordExp (A_loc loc, S_symbol name, A_efieldList fields)
{
    A_exp p = checked_malloc (sizeof (*p));
    p->kind = A_recordExp;
    p->loc = *loc;
    p->u.record.name = name;
    p->u.record.fields = fields;
    return p;
}

A_exp A_SeqExp (A_loc loc, A_expList seq)
{
    A_exp p = checked_malloc (sizeof (*p));
    p->kind = A_seqExp;
    p->loc = *loc;
    p->u.seq = seq;
    return p;
}

A_exp A_AssignExp (A_loc loc, A_var var, A_exp exp)
{
    A_exp p = checked_malloc (sizeof (*p));
    p->kind = A_assignExp;
    p->loc = *loc;
    p->u.assign.var = var;
    p->u.assign.exp = exp;
    return p;
}

A_exp A_IfExp (A_loc loc, A_exp test, A_scope tr, A_scope fl)
{
    A_exp p = checked_malloc (sizeof (*p));
    p->kind = A_ifExp;
    p->loc = *loc;
    p->u.iff.test = test;
    p->u.iff.tr = tr;
    p->u.iff.fl = fl;
    return p;
}

A_exp A_WhileExp (A_loc loc, A_exp test, A_scope body)
{
    A_exp p = checked_malloc (sizeof (*p));
    p->kind = A_whileExp;
    p->loc = *loc;
    p->u.whilee.test = test;
    p->u.whilee.body = body;
    return p;
}

A_exp A_ForExp (A_loc loc, S_symbol var, A_exp lo, A_exp hi, A_scope body)
{
    A_exp p = checked_malloc (sizeof (*p));
    p->kind = A_forExp;
    p->loc = *loc;
    p->u.forr.var = var;
    p->u.forr.lo = lo;
    p->u.forr.hi = hi;
    p->u.forr.body = body;
    p->u.forr.escape = TRUE;
    return p;
}

A_exp A_BreakExp (A_loc loc)
{
    A_exp p = checked_malloc (sizeof (*p));
    p->kind = A_breakExp;
    p->loc = *loc;
    return p;
}

A_exp A_RetExp (A_loc loc, A_exp exp)
{
    A_exp p = checked_malloc (sizeof (*p));
    p->kind = A_retExp;
    p->u.ret = exp;
    p->loc = *loc;
    return p;
}

A_exp A_ArrayExp (A_loc loc, A_expList list)
{
    A_exp p = checked_malloc (sizeof (*p));
    p->kind = A_arrayExp;
    p->loc = *loc;
    p->u.array = list;
    return p;
}

A_expList A_ExpList (A_exp head, A_expList tail)
{
    A_expList p = checked_malloc (sizeof (*p));
    p->head = head;
    p->tail = tail;
    return p;
}

/******************
 *  Declarations  *
 ******************/

A_dec A_FunctionDec (A_loc loc, S_symbol name, A_fieldList params, A_ty type, A_scope scope)
{
    A_dec p = checked_malloc (sizeof (*p));
    p->kind = A_functionDec;
    p->loc = *loc;
    p->u.function.name = name;
    p->u.function.params = params;
    p->u.function.type = type;
    p->u.function.scope = scope;
    return p;
}

A_dec A_VarDec (A_loc loc, S_symbol var, A_ty type, A_exp init)
{
    A_dec p = checked_malloc (sizeof (*p));
    p->kind = A_varDec;
    p->u.var.var = var;
    p->u.var.type = type;
    p->u.var.init = init;
    p->u.var.escape = TRUE;
    p->loc = *loc;
    return p;
}

A_dec A_TypeDec (A_loc loc, S_symbol name, A_ty type)
{
    A_dec p = checked_malloc (sizeof (*p));
    p->kind = A_typeDec;
    p->loc = *loc;
    p->u.type.name = name;
    p->u.type.type = type;
    return p;
}

A_decList A_DecList (A_dec head, A_decList tail)
{
    A_decList p = checked_malloc (sizeof (*p));
    p->head = head;
    p->tail = tail;
    return p;
}

/***********
 *  Types  *
 ***********/

A_ty A_NameTy (A_loc loc, S_symbol name, A_specList specs)
{
    A_ty p = checked_malloc (sizeof (*p));
    p->kind = A_nameTy;
    p->u.name = name;
    p->specs = specs;
    p->loc = *loc;
    return p;
}

A_ty A_PointerTy (A_loc loc, A_ty type)
{
    A_ty p = checked_malloc (sizeof (*p));
    p->kind = A_pointerTy;
    p->u.pointer = type;
    p->loc = *loc;
    return p;
}

A_ty A_ArrayTy (A_loc loc, A_ty type, A_exp size)
{
    A_ty p = checked_malloc (sizeof (*p));
    p->loc = *loc;
    p->kind = A_arrayTy;
    p->u.array.type = type;
    p->u.array.size = size;
    return p;
}

A_ty A_RecordTy (A_loc loc, A_fieldList record)
{
    A_ty p = checked_malloc (sizeof (*p));
    p->kind = A_recordTy;
    p->loc = *loc;
    p->u.record = record;
    return p;
}

/**************
 *  Literals  *
 **************/

A_literal A_LiteralBool (A_loc loc, bool value)
{
    A_literal p = checked_malloc (sizeof (*p));
    p->kind = A_literalBool;
    p->u.boolean = value;
    p->loc = *loc;
    return p;
}

A_literal A_LiteralInt (A_loc loc, int value)
{
    A_literal p = checked_malloc (sizeof (*p));
    p->kind = A_literalInt;
    p->u.integer = value;
    p->loc = *loc;
    return p;
}

A_literal A_LiteralFloat (A_loc loc, double value)
{
    A_literal p = checked_malloc (sizeof (*p));
    p->kind = A_literalFloat;
    p->u.fp = value;
    p->loc = *loc;
    return p;
}

A_literal A_LiteralString (A_loc loc, const char * value)
{
    A_literal p = checked_malloc (sizeof (*p));
    p->kind = A_literalString;
    p->u.string = value;
    p->loc = *loc;
    return p;
}

/****************
 *  Statements  *
 ****************/

A_stm A_StmExp (A_exp exp)
{
    A_stm p = checked_malloc (sizeof (*p));
    p->kind = A_stmExp;
    p->u.exp = exp;
    return p;
}

A_stm A_StmDec (A_dec dec)
{
    A_stm p = checked_malloc (sizeof (*p));
    p->kind = A_stmDec;
    p->u.dec = dec;
    return p;
}

A_stmList A_StmList (A_stm head, A_stmList tail)
{
    A_stmList p = checked_malloc (sizeof (*p));
    p->head = head;
    p->tail = tail;
    return p;
}

/***********
 *  Scope  *
 ***********/

A_scope A_Scope (A_stmList list)
{
    A_scope p = checked_malloc (sizeof (*p));
    p->list = list;
    return p;
}

/***********
 *  Specs  *
 ***********/

A_spec A_SpecType (A_loc loc, A_ty type)
{
    A_spec p = checked_malloc (sizeof (*p));
    p->kind = A_specType;
    p->u.type = type;
    p->loc = *loc;
    return p;
}

A_spec A_SpecLiteral (A_loc loc, A_literal literal)
{
    A_spec p = checked_malloc (sizeof (*p));
    p->kind = A_specLiteral;
    p->u.literal = literal;
    p->loc = *loc;
    return p;
}

A_specList A_SpecList (A_spec head, A_specList tail)
{
    A_specList p = checked_malloc (sizeof (*p));
    p->head = head;
    p->tail = tail;
    return p;
}

/*************
 *  Printer  *
 *************/

static char str_oper[][12] =
{
    "PLUS", "MINUS", "TIMES", "DIVIDE",
    "EQUAL", "NOTEQUAL", "LESSTHAN", "LESSEQ", "GREAT", "GREATEQ"
};

static void PrintExp (FILE * out, A_exp v, int d);
static void PrintVar (FILE * out, A_var v, int d);
static void PrintDec (FILE * out, A_dec v, int d);
static void PrintType (FILE * out, A_ty v, int d);
static void PrintField (FILE * out, A_field v, int d);
static void PrintFieldList (FILE * out, A_fieldList v, int d);
static void PrintExpList (FILE * out, A_expList v, int d);
static void PrintExpField (FILE * out, A_efield v, int d);
static void PrintExpFieldList (FILE * out, A_efieldList v, int d);

void AST_Print (FILE * out, A_decList list, int d)
{
    fprintf (out, "Declarations(");
    LIST_FOREACH (dec, list)
    {
        fprintf (out, "\n");
        PrintDec (out, dec, d + 1);
    }
    fprintf (out, ")\n");
}

static void PrintIndent (FILE * out, int d)
{
    int i;

    for (i = 0; i < d; i++)
    {
        fprintf (out, "  ");
    }
}

static void PrintScope (FILE * out, A_scope scope, int d)
{
    PrintIndent (out, d);

    fprintf (out, "Scope(");

    if (!scope)
    {
        fprintf (out, ")");
        return;
    }
    else
    {
        fprintf (out, "\n");
    }

    size_t size = LIST_SIZE (scope->list);
    LIST_FOREACH (stm, scope->list)
    {
        switch (stm->kind)
        {
        case A_stmDec:
        {
            PrintDec (out, stm->u.dec, d + 1);
            break;
        }
        case A_stmExp:
        {
            PrintExp (out, stm->u.exp, d + 1);
            break;
        }
        }

        size--;

        if (size)
        {
            fprintf (out, ",\n");
        }
    }

    fprintf (out, ")");
}

static void PrintLiteral (FILE * out, A_literal literal, int d)
{
    PrintIndent (out, d);

    fprintf (out, "Literal(");

    switch (literal->kind)
    {
    case A_literalBool:
    {
        fprintf (out, "Boolean(%s)", literal->u.boolean ? "True" : "False");
        break;
    }
    case A_literalInt:
    {
        fprintf (out, "Integer(%d)", literal->u.integer);
        break;
    }
    case A_literalFloat:
    {
        fprintf (out, "Float(%f)", literal->u.fp);
        break;
    }
    case A_literalString:
    {
        fprintf (out, "String(%s)", literal->u.string);
        break;
    }
    }
}

static void PrintSpecs (FILE * out, A_specList specs, int d)
{
    PrintIndent (out, d);

    fprintf (out, "Specs(\n");

    size_t size = LIST_SIZE (specs);
    LIST_FOREACH (spec, specs)
    {
        switch (spec->kind)
        {
        case A_specType:
        {
            PrintType (out, spec->u.type, d + 1);
            break;
        }
        case A_specLiteral:
        {
            PrintLiteral (out, spec->u.literal, d + 1);
            break;
        }
        }

        size--;

        if (size)
        {
            fprintf (out, ",\n");
        }
    }

    fprintf (out, ")");
}

/* Print A_var types. PrintIndent d spaces. */
static void PrintVar (FILE * out, A_var v, int d)
{
    PrintIndent (out, d);

    switch (v->kind)
    {
    case A_simpleVar:
        fprintf (out, "SimpleVar(%s)", S_Name (v->u.simple));
        break;

    case A_fieldVar:
        fprintf (out, "%s\n", "FieldVar(");
        PrintVar (out, v->u.field.var, d + 1);
        fprintf (out, "%s\n", ",");
        PrintIndent (out, d + 1);
        fprintf (out, "%s)", S_Name (v->u.field.sym));
        break;

    case A_subscriptVar:
        fprintf (out, "%s\n", "SubscriptVar(");
        PrintVar (out, v->u.subscript.var, d + 1);
        fprintf (out, "%s\n", ",");
        PrintExp (out, v->u.subscript.exp, d + 1);
        fprintf (out, "%s", ")");
        break;

    default:
        assert (0);
    }
}

static void PrintOp (FILE * out, A_oper d)
{
    fprintf (out, "%s", str_oper[d]);
}

static void PrintExp (FILE * out, A_exp v, int d)
{
    PrintIndent (out, d);

    switch (v->kind)
    {
    case A_asmExp:
        fprintf (out, "AsmExp(%s, %s",
                 v->u.assembly.code,
                 v->u.assembly.data);
        break;

    case A_retExp:
        fprintf (out, "RetExp(\n");
        PrintExp (out, v->u.ret, d + 1);
        fprintf (out, ")");
        break;

    case A_macroCallExp:
        fprintf (out, "MacroCallExp(%s,\n", S_Name (v->u.call.func));
        PrintExpList (out, v->u.call.args, d + 1);
        fprintf (out, ")");
        break;

    case A_varExp:
        fprintf (out, "VarExp(");
        PrintVar (out, v->u.var, 0);
        fprintf (out, "%s", ")");
        break;

    case A_nilExp:
        fprintf (out, "NilExp()");
        break;

    case A_intExp:
        fprintf (out, "IntExp(%d)", v->u.intt);
        break;

    case A_stringExp:
        fprintf (out, "StringExp(%s)", v->u.stringg);
        break;

    case A_callExp:
        fprintf (out, "CallExp(%s,\n", S_Name (v->u.call.func));
        PrintExpList (out, v->u.call.args, d + 1);
        fprintf (out, ")");
        break;

    case A_opExp:
        fprintf (out, "OpExp(");
        PrintOp (out, v->u.op.oper);
        fprintf (out, ",\n");
        PrintExp (out, v->u.op.left, d + 1);
        fprintf (out, ",\n");
        PrintExp (out, v->u.op.right, d + 1);
        fprintf (out, ")");
        break;

    case A_recordExp:
        fprintf (out, "RecordExp(%s,\n", v->u.record.name ? S_Name (v->u.record.name) : "Anonymous");
        PrintExpFieldList (out, v->u.record.fields, d + 1);
        fprintf (out, ")");
        break;

    case A_seqExp:
        fprintf (out, "SeqExp(\n");
        PrintExpList (out, v->u.seq, d + 1);
        fprintf (out, ")");
        break;

    case A_assignExp:
        fprintf (out, "AssignExp(\n");
        PrintVar (out, v->u.assign.var, d + 1);
        fprintf (out, ",\n");
        PrintExp (out, v->u.assign.exp, d + 1);
        fprintf (out, ")");
        break;

    case A_ifExp:
        fprintf (out, "IfExp(\n");
        PrintExp (out, v->u.iff.test, d + 1);
        fprintf (out, ",\n");
        PrintScope (out, v->u.iff.tr, d + 1);

        if (v->u.iff.fl)   /* else is optional */
        {
            fprintf (out, ",\n");
            PrintScope (out, v->u.iff.fl, d + 1);
        }

        fprintf (out, ")");
        break;

    case A_whileExp:
        fprintf (out, "WhileExp(\n");
        PrintExp (out, v->u.whilee.test, d + 1);
        fprintf (out, ",\n");
        PrintScope (out, v->u.whilee.body, d + 1);
        fprintf (out, ")\n");
        break;

    case A_forExp:
        fprintf (out, "ForExp(%s,\n", S_Name (v->u.forr.var));
        PrintExp (out, v->u.forr.lo, d + 1);
        fprintf (out, ",\n");
        PrintExp (out, v->u.forr.hi, d + 1);
        fprintf (out, "%s\n", ",");
        PrintScope (out, v->u.forr.body, d + 1);
        fprintf (out, ",\n");
        PrintIndent (out, d + 1);
        fprintf (out, "%s", v->u.forr.escape ? "TRUE)" : "FALSE)");
        break;

    case A_breakExp:
        fprintf (out, "BreakExp()");
        break;

    case A_arrayExp:
        fprintf (out, "ArrayExp(\n");
        PrintExpList (out, v->u.array, d + 1);
        fprintf (out, ")");
        break;

    default:
        assert (0);
    }
}

static void PrintDec (FILE * out, A_dec v, int d)
{
    PrintIndent (out, d);

    switch (v->kind)
    {
    case A_functionDec:
    {
        fprintf (out, "FunDec(%s,", S_Name (v->u.function.name));
        if (v->u.function.type)
        {
            fprintf (out, "\n");
            PrintType (out, v->u.function.type, d + 1);
            fprintf (out, ",\n");
        }
        else
        {
            fprintf (out, "auto,\n");
        }

        PrintFieldList (out, v->u.function.params, d + 1);
        fprintf (out, ",\n");

        PrintScope (out, v->u.function.scope, d + 1);
        fprintf (out, ")");
        break;
    }

    case A_varDec:
    {
        if (v->u.var.type)
        {
            fprintf (out, "VarDec(%s,\n", S_Name (v->u.var.var));
            PrintType (out, v->u.var.type, d + 1);
        }
        else
        {
            fprintf (out, "VarDec(%s,auto", S_Name (v->u.var.var));
        }


        if (v->u.var.init)
        {
            fprintf (out, ",\n");
            PrintExp (out, v->u.var.init, d + 1);
        }

        fprintf (out, ")");
        break;
    }

    case A_typeDec:
    {
        fprintf (out, "TypeDec(%s,\n", S_Name (v->u.type.name));
        PrintType (out, v->u.type.type, d + 1);
        fprintf (out, ")");
        break;
    }

    default:
        assert (0);
    }
}

static void PrintType (FILE * out, A_ty v, int d)
{
    PrintIndent (out, d);

    switch (v->kind)
    {
    case A_nameTy:
        fprintf (out, "NameTy(%s", S_Name (v->u.name));
        break;

    case A_recordTy:
        fprintf (out, "RecordTy(\n");
        PrintFieldList (out, v->u.record, d + 1);
        break;

    case A_arrayTy:
        fprintf (out, "ArrayTy(\n");
        PrintType (out, v->u.array.type, d + 1);
        fprintf (out, ",\n");
        PrintExp (out, v->u.array.size, d + 1);
        break;

    default:
        assert (0);
    }

    if (v->specs)
    {
        fprintf (out, ",\n");
        PrintSpecs (out, v->specs, d + 1);
    }
    fprintf (out, ")");
}

static void PrintField (FILE * out, A_field v, int d)
{
    (void) d;
    fprintf (out, "Field(%s,\n", S_Name (v->name));
    PrintType (out, v->type, d + 1);
    fprintf (out, ")");
}

static void PrintFieldList (FILE * out, A_fieldList v, int d)
{
    PrintIndent (out, d);

    if (v)
    {
        fprintf (out, "FieldList(");
        PrintField (out, v->head, d);
        fprintf (out, ",\n");
        PrintFieldList (out, v->tail, d + 1);
        fprintf (out, ")");
    }
    else
    {
        fprintf (out, "FieldList()");
    }
}

static void PrintExpList (FILE * out, A_expList v, int d)
{
    PrintIndent (out, d);

    if (v)
    {
        fprintf (out, "ExpList(\n");
        PrintExp (out, v->head, d + 1);
        fprintf (out, ",\n");
        PrintExpList (out, v->tail, d + 1);
        fprintf (out, ")");
    }
    else
    {
        fprintf (out, "ExpList()");
    }

}

static void PrintExpField (FILE * out, A_efield v, int d)
{
    PrintIndent (out, d);

    if (v)
    {
        fprintf (out, "Efield(%s,\n", S_Name (v->name));
        PrintExp (out, v->exp, d + 1);
        fprintf (out, ")");
    }
    else
    {
        fprintf (out, "Efield()");
    }
}

static void PrintExpFieldList (FILE * out, A_efieldList v, int d)
{
    PrintIndent (out, d);

    if (v)
    {
        fprintf (out, "EFieldList(\n");
        PrintExpField (out, v->head, d + 1);
        fprintf (out, ",\n");
        PrintExpFieldList (out, v->tail, d + 1);
        fprintf (out, ")");
    }
    else
    {
        fprintf (out, "EFieldList()");
    }
}
