#include <stdlib.h>
#include <stdio.h>

#include "ext/vector.h"

#include "symbol.h"
#include "ast.h"
#include "errormsg.h"
#include "parse.h"
#include "program.h"

extern FILE * yyin;
extern void * yy_scan_string (const char * str);
extern int yyparse (void);
extern A_decList ast_root;

int Parse_File (Program_Module m, string fname)
{
    EM_reset (fname);

    yyin = fopen (fname, "r");

    if (!yyin)
    {
        Vector_PushBack (m->errors.lexer, "Could not open file");
        return 1;
    }

    ast_root = NULL;
    if (yyparse() != 0)
    {
        m->ast = NULL;
        return 1;
    }

    m->ast = ast_root;

    return 0;
}

int Parse_String (Program_Module m, const char * input)
{
    if (!input)
    {
        Vector_PushBack (m->errors.lexer, "Empty input");
        return 1;
    }

    EM_reset ("inputing");
    yy_scan_string (input);

    ast_root = NULL;
    if (yyparse() != 0)
    {
        m->ast = NULL;
    }

    m->ast = ast_root;

    return 0;
}
