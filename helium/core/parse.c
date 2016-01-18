#include <stdlib.h>
#include <stdio.h>

#include "util/vector.h"

#include "modules/mips/ast.h"

#include "core/symbol.h"
#include "core/program.h"
#include "core/parse.h"

extern FILE * yy_helium_in;
extern void * yy_helium__scan_string (const char * str);
extern void * yy_helium__get_current_buffer();
extern int HeliumParse (Program_Module m);

A_asmStmList yy_mips_result;
extern void * yy_mips__scan_string (const char * str);
extern int MIPSParse (A_loc loc);

// HMM... ca we make it parse from the same buffer as helium parser. Do we need this?
A_asmStmList ParseAsm(A_loc loc, const char * input)
{
    yy_mips__scan_string (input);
    int status = MIPSParse(loc);
    return status ? NULL : yy_mips_result;
}

int Parse_File (Program_Module m, String filename)
{
    yy_helium_in = fopen (String_Data (filename), "r");

    if (!yy_helium_in)
    {
        Vector_PushBack (&m->errors.lexer, "Could not open file");
        return 1;
    }

    return HeliumParse (m) || !Vector_Empty (&m->errors.lexer) || !Vector_Empty (&m->errors.parser);
}

int Parse_String (Program_Module m, const char * input)
{
    if (!input)
    {
        Vector_PushBack (&m->errors.lexer, "Empty input");
        return 1;
    }

    yy_helium__scan_string (input);

    return HeliumParse (m) || !Vector_Empty (&m->errors.lexer) || !Vector_Empty (&m->errors.parser);
}
