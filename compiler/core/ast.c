#include <assert.h>
#include <inttypes.h>

#include "util/mem.h"
#include "core/ast.h"

/**************
 *  Literals  *
 **************/

A_literal A_LiteralBool (A_loc loc, bool value)
{
    A_literal p = checked_malloc (sizeof * p);
    p->loc = *loc;
    p->kind = A_literalBool;
    p->u.bval = value;
    return p;
}

A_literal A_LiteralInt (A_loc loc, intmax_t value)
{
    A_literal p = checked_malloc (sizeof * p);
    p->loc = *loc;
    p->kind = A_literalInt;
    p->u.ival = value;
    return p;
}

A_literal A_LiteralUInt (A_loc loc, uintmax_t value)
{
    A_literal p = checked_malloc (sizeof * p);
    p->loc = *loc;
    p->kind = A_literalUInt;
    p->u.uval = value;
    return p;
}

A_literal A_LiteralFloat (A_loc loc, double value)
{
    A_literal p = checked_malloc (sizeof * p);
    p->loc = *loc;
    p->kind = A_literalFloat;
    p->u.dval = value;
    return p;
}

A_literal A_LiteralString (A_loc loc, const char * value)
{
    A_literal p = checked_malloc (sizeof * p);
    p->loc = *loc;
    p->kind = A_literalString;
    p->u.sval = value;
    return p;
}

bool A_LiteralInRange_i (A_literal lit, intmax_t min, intmax_t max)
{
    assert (lit->kind == A_literalInt);
    return lit->u.ival >= min && lit->u.ival <= max;
}

bool A_LiteralInRange_u (A_literal lit, uintmax_t min, uintmax_t max)
{
    assert (lit->kind == A_literalInt);
    return lit->u.uval >= min && lit->u.uval <= max;
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

void AST_PrintLiteral (FILE * out, A_literal lit, int d)
{
    AST_PrintIndent (out, d);

    fprintf (out, "Lit(");

    switch (lit->kind)
    {
    case A_literalBool:
    {
        fprintf (out, "Bool(%s)", lit->u.bval ? "true" : "false");
        break;
    }
    case A_literalInt:
    {
        fprintf (out, "Int(%" PRIdMAX ")", lit->u.ival);
        break;
    }
    case A_literalUInt:
    {
        fprintf (out, "UInt(%" PRIuMAX ")", lit->u.uval);
        break;
    }
    case A_literalFloat:
    {
        fprintf (out, "Float(%f)", lit->u.dval);
        break;
    }
    case A_literalString:
    {
        fprintf (out, "String(%s)", lit->u.sval);
        break;
    }
    }
}
