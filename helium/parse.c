#include <stdlib.h>
#include <stdio.h>

#include "ext/vector.h"

#include "symbol.h"
#include "parse.h"
#include "program.h"
#include "ast.h"

extern FILE * yyin;
extern void * yy_scan_string (const char * str);
extern int Parse (Program_Module m);

int Parse_File (Program_Module m, String filename)
{
    yyin = fopen (String_Data(filename), "r");

    if (!yyin)
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

    yy_scan_string (input);

    return Parse (m) || !Vector_Empty (&m->errors.lexer) || !Vector_Empty (&m->errors.parser);
}
