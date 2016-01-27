#ifndef AST_H_50UWFKJ9
#define AST_H_50UWFKJ9

#include <stdio.h>
#include <stdint.h>

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

    // FIXME append Kind
    enum
    {
        A_literalBool,
        A_literalInt,
        A_literalUInt,
        A_literalFloat,
        A_literalString
    } kind;

    struct
    {
        bool         bval;
        intmax_t     ival;
        uintmax_t    uval;
        double       dval;
        const char * sval;
    } u;

} * A_literal;

A_literal A_LiteralBool (A_loc loc, bool value);
A_literal A_LiteralInt (A_loc loc, intmax_t value);
A_literal A_LiteralUInt (A_loc loc, uintmax_t value);
A_literal A_LiteralFloat (A_loc loc, double value);
A_literal A_LiteralString (A_loc loc, const char * value);

// TODO implement raw literals
A_literal A_LiteralRawBin (A_loc loc, const char * value);
A_literal A_LiteralRawOct (A_loc loc, const char * value);
A_literal A_LiteralRawHex (A_loc loc, const char * value);
A_literal A_LiteralRawDec (A_loc loc, const char * value);
A_literal A_LiteralRawFloat (A_loc loc, const char * value);

#define A_LiteralIsBool(l) (l->kind == A_literalBool)
#define A_LiteralIsInteger(l) (l->kind == A_literalInt || l->kind == A_literalUInt)
#define A_LiteralIsSignedInteger(l) (l->kind == A_literalInt)
#define A_LiteralIsFloat(l) (l->kind == A_literalFloat)
#define A_LiteralIsString(l) (l->kind == A_literalString)

bool A_LiteralInRange_i (A_literal lit, intmax_t min, intmax_t max);
bool A_LiteralInRange_u (A_literal lit, uintmax_t min, uintmax_t max);
#define A_LiteralInRange(l,a,b)                                                \
    _Generic ((0,a),                                                           \
            int8_t:    A_LiteralInRange_i,                                     \
            int16_t:   A_LiteralInRange_i,                                     \
            int32_t:   A_LiteralInRange_i,                                     \
            int64_t:   A_LiteralInRange_i,                                     \
            intmax_t:  A_LiteralInRange_i,                                     \
            uint8_t:   A_LiteralInRange_u,                                     \
            uint16_t:  A_LiteralInRange_u,                                     \
            uint32_t:  A_LiteralInRange_u,                                     \
            uint64_t:  A_LiteralInRange_u,                                     \
            uintmax_t: A_LiteralInRange_u                                      \
            )(l,a,b)

/*************
*  Printer  *
*************/

void AST_PrintIndent (FILE * out, int d);
void AST_PrintLiteral (FILE * out, A_literal lit, int d);

#endif /* end of include guard: AST_H_50UWFKJ9 */
