#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ext/vector.h"
#include "ext/list.h"
#include "ext/util.h"
#include "ext/mem.h"

#include "program.h"
#include "regalloc.h"

Program_Module Program_ModuleNew()
{
    Program_Module r = checked_malloc (sizeof * r);

    r->input = NULL;
    r->ast = NULL;
    r->fragments.strings = NULL;
    r->fragments.functions = NULL;

    r->errors.lexer = Vector_New (const char *);
    r->errors.parser = Vector_New (const char *);
    r->errors.semant = Vector_New (const char *);

    r->results = Vector_New (RA_Result);

    return r;
}

void Program_AddFragment (Program_Module p, F_frag f)
{
    assert (p);
    assert (f);

    switch (f->kind)
    {
    case F_stringFrag:
        LIST_PUSH (p->fragments.strings, f);
        break;

    case F_procFrag:
        LIST_PUSH (p->fragments.functions, f);
        break;

    default:
        assert (0);
        break;
    }
}

void Program_PrintAssembly (FILE * file, Program_Module p)
{
    assert (file);
    assert (p);

    if (p->fragments.strings)
    {
        fprintf (file, ".data\n");
        LIST_FOREACH (fragment, p->fragments.strings)
        {
            fprintf (file, "%s: .asciiz \"%s\"\n", fragment->u.str.label->name, fragment->u.str.str);
        }
        fprintf (file, "\n");
    }
    fprintf (file, ".text\n");
    fprintf (file, ".globl main\n");
    VECTOR_FOREACH (RA_Result, r, p->results)
    {
        LIST_FOREACH (line, (*r)->il)
        {
            if (line->kind != I_META)
            {
                ASM_PrintLine (file, line, Temp_LayerMap ((*r)->coloring, Temp_Name()));
            }
        }
    }
}
