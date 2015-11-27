#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ext/mem.h"
#include "ext/vector.h"
#include "ext/list.h"

#include "program.h"
#include "parse.h"
#include "translate.h"
#include "mipsmachine.h"
#include "semant.h"
#include "canon.h"
#include "codegen.h"
#include "regalloc.h"
#include "error.h"

static const char * file_drop_extension (const char * file)
{
    const char * dot = strrchr (file, '.');
    if (dot)
    {
        return strncpy (checked_malloc (strlen (file)), file, dot - file);
    }
    else
    {
        return file;
    }
}

int main (int argc, char * argv[])
{
    char * filename = argv[1];

    Program_Module m = Program_ModuleNew();
    Program_ParseArguments (m, argc, argv);

    printf ("Compiling %s\n", String_Data (m->options.file));

    if (Parse_File (m, m->options.file) != 0)
    {
        printf ("Lex errors: %lu\n", Vector_Size (&m->errors.lexer));
        VECTOR_FOREACH (struct Error, error, &m->errors.lexer)
        {
            Error_Print (stdout, error);
        }

        printf ("Parse errors: %lu\n", Vector_Size (&m->errors.parser));
        VECTOR_FOREACH (struct Error, error, &m->errors.parser)
        {
            Error_Print (stdout, error);
        }

        exit (1);
    }

    MIPS_Init();

    F_Init();

    if (Semant_Translate (m) != 0)
    {
        printf ("Failed to translate program\n");
        printf ("Semant errors: %lu\n", Vector_Size (&m->errors.semant));
        VECTOR_FOREACH (struct Error, error, &m->errors.semant)
        {
            Error_Print (stdout, error);
        }

        exit (1);
    }

    LIST_FOREACH (fragment, m->fragments.functions)
    {
        T_stmList sl = C_Linearize (fragment->u.proc.body);
        sl = C_TraceSchedule (C_BasicBlocks (sl));
        ASM_lineList lines = F_CodeGen (fragment->u.proc.frame, sl);
        /* ASM_PrintLineList (stdout, lines, Temp_LayerMap (F_tempMap, Temp_Name ())); */
        F_frame frame = fragment->u.proc.frame;
        lines = F_ProcEntryExit2 (frame, lines);
        RA_Result rar = RA_RegAlloc (frame, lines, regs_all, regs_colors);
        lines = F_ProcEntryExit3 (frame, lines, rar->colors);

        Vector_PushBack (&m->results, rar);

    }

    char * outfile = checked_malloc (strlen (filename) + 3);
    sprintf (outfile, "%s.s", file_drop_extension (filename));

    FILE * file = fopen (outfile, "w");

    Program_PrintAssembly (file, m);

    return 0;
}
