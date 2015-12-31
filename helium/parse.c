#include <stdlib.h>
#include <stdio.h>

#include "ext/vector.h"

#include "symbol.h"
#include "program.h"

#include "parse.h"

extern FILE * yy_helium_in;
extern void * yy_helium__scan_string (const char * str);
extern void * yy_helium__get_current_buffer();
extern int HeliumParse (Program_Module m);

A_asmStmList yy_mips_result;
extern void * yy_mips__scan_string (const char * str);
extern int MIPSParse ();

// HMM... ca we make it parse from the same buffer as helium parser. Do we need this?
A_asmStmList ParseAsm(const char * input)
{
    printf("parse asm: '%s'\n", input);
    yy_mips__scan_string (input);
    int status = MIPSParse();
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
