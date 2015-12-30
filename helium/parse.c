#include <stdlib.h>
#include <stdio.h>

#include "ext/vector.h"

#include "symbol.h"
#include "parse.h"
#include "program.h"
#include "ast.h"

extern FILE * yy_helium_in;
extern void * yy_helium__scan_string (const char * str);
extern int Parse (Program_Module m);

int Parse_File (Program_Module m, String filename)
{
    yy_helium_in = fopen (String_Data(filename), "r");

    if (!yy_helium_in)
    {
        Vector_PushBack (&m->errors.lexer, "Could not open file");
        return 1;
    }

    return Parse (m) || !Vector_Empty (&m->errors.lexer) || !Vector_Empty (&m->errors.parser);
}

int Parse_String (Program_Module m, const char * input)
{
    if (!input)
    {
        Vector_PushBack (&m->errors.lexer, "Empty input");
        return 1;
    }

    yy_helium__scan_string (input);

    return Parse (m) || !Vector_Empty (&m->errors.lexer) || !Vector_Empty (&m->errors.parser);
}
