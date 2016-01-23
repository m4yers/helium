#ifndef AST_H_50UWFKJ9
#define AST_H_50UWFKJ9

#include <stdio.h>

#include "util/bool.h"

/**************
 *  Location  *
 **************/

typedef struct A_loc_t
{
    int first_line;
    int first_column;
    int last_line;
    int last_column;
    //NOTE token MUST NOT be used outside lexer and parser
    const char * token;
} * A_loc;

/**************
 *  Literals  *
 **************/

typedef struct A_literal_t
{
    struct A_loc_t loc;

    enum
    {
        A_literalBool,
        A_literalInt,
        A_literalHex,
        A_literalFloat,
        A_literalString
    } kind;

    const char * string;

} * A_literal;

A_literal A_LiteralBool (A_loc loc, const char * value);
A_literal A_LiteralInt (A_loc loc, const char * value);
A_literal A_LiteralHex (A_loc loc, const char * value);
A_literal A_LiteralFloat (A_loc loc, const char * value);
A_literal A_LiteralString (A_loc loc, const char * value);

/*************
*  Printer  *
*************/

void AST_PrintIndent (FILE * out, int d);
void AST_PrintLiteral (FILE * out, A_literal literal, int d);

#endif /* end of include guard: AST_H_50UWFKJ9 */
