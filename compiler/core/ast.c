#include "util/mem.h"
#include "core/ast.h"

/**************
 *  Literals  *
 **************/

A_literal A_LiteralBool (A_loc loc, const char * value)
{
    A_literal p = checked_malloc (sizeof * p);
    p->kind = A_literalString;
    p->string = value;
    p->loc = *loc;
    return p;
}

A_literal A_LiteralInt (A_loc loc, const char * value)
{
    A_literal p = checked_malloc (sizeof * p);
    p->kind = A_literalString;
    p->string = value;
    p->loc = *loc;
    return p;
}

A_literal A_LiteralHex (A_loc loc, const char * value)
{
    A_literal p = checked_malloc (sizeof * p);
    p->kind = A_literalString;
    p->string = value;
    p->loc = *loc;
    return p;
}

A_literal A_LiteralFloat (A_loc loc, const char * value)
{
    A_literal p = checked_malloc (sizeof * p);
    p->kind = A_literalString;
    p->string = value;
    p->loc = *loc;
    return p;
}

A_literal A_LiteralString (A_loc loc, const char * value)
{
    A_literal p = checked_malloc (sizeof * p);
    p->kind = A_literalString;
    p->string = value;
    p->loc = *loc;
    return p;
}

/*************
*  Printer  *
*************/

void AST_PrintIndent (FILE * out, int d)
{
    int i;

    for (i = 0; i < d; i++)
    {
        fprintf (out, "  ");
    }
}

void AST_PrintLiteral (FILE * out, A_literal literal, int d)
{
    AST_PrintIndent (out, d);

    fprintf (out, "Literal(");

    switch (literal->kind)
    {
    case A_literalBool:
    {
        fprintf (out, "Bool(%s)", literal->string);
        break;
    }
    case A_literalInt:
    {
        fprintf (out, "Int(%s)", literal->string);
        break;
    }
    case A_literalHex:
    {
        fprintf (out, "Hex(%s)", literal->string);
        break;
    }
    case A_literalFloat:
    {
        fprintf (out, "Float(%s)", literal->string);
        break;
    }
    case A_literalString:
    {
        fprintf (out, "String(%s)", literal->string);
        break;
    }
    }
}
