#define TOTAL_ELEMENTS(array) (sizeof(array) / sizeof(array[0]))

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "ext/list.h"
#include "ext/vector.h"

#include "symbol.h"
#include "parse.h"
#include "env.h"
#include "preproc.h"
#include "semant.h"
#include "translate.h"
#include "canon.h"
#include "codegen.h"
#include "regalloc.h"
#include "mipsmachine.h"
#include "escape.h"
#include "error.h"

#include "program.h"

static void run_cases (void ** state, const char * cases[], size_t len)
{
    for (size_t i = 0; i < len; ++i)
    {
        Program_Module m = Program_ModuleNew();

        /* printf ("Program:\n%s\n", cases[i]); */

        if (Parse_String (m, cases[i]) != 0)
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

            fail();
        }

        MIPS_Init();
        F_Init();

        assert_true (m->ast);

        /* AST_Print (stdout, m->ast, 0); */
        /* exit(0); */

        if (PreProc_Translate (m) != 0)
        {
            printf ("Failed to pre process program\n");
            printf ("Preprocessor  errors: %lu\n", Vector_Size (&m->errors.preproc));
            VECTOR_FOREACH (struct Error, error, &m->errors.preproc)
            {
                Error_Print (stdout, error);
            }
            fail();
        }

        /* AST_Print (stdout, m->ast, 0); */
        /* exit (0); */

        if (Semant_Translate (m) != 0)
        {
            printf ("Failed to translate program\n");
            printf ("Semant errors: %lu\n", Vector_Size (&m->errors.semant));
            VECTOR_FOREACH (struct Error, error, &m->errors.semant)
            {
                Error_Print (stdout, error);
            }
            fail();
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

        Program_PrintAssembly (stdout, m);
    }

    (void) state; /* unused */
}

static void macro__panic (void ** state)
{
    const char * cases[] =
    {
        "fn main\n\
         {\n\
             panic!(\"Oh no!\");\n\
         }",
    };

    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

static void macro__assert (void ** state)
{
    const char * cases[] =
    {
        "fn main\n\
         {\n\
             assert!(1);\n\
         }",
    };

    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

static void main__return (void ** state)
{
    const char * cases[] =
    {
        /* "fn main\n\ */
        /*  {\n\ */
        /*      fn blah { 1; }\n\ */
        /*  }", */
        /* "fn main\n\ */
        /*  {\n\ */
        /*      ret \"a string\";\n\ */
        /*  }", */
        /* "fn main\n\ */
        /* {\n\ */
        /*     fn plus (a: int, b: int)\n\ */
        /*     {\n\ */
        /*         fn get_a { a; }\n\ */
        /*         fn get_b { ret b; }\n\ */
        /*         ret 0;\n\ */
        /*     }\n\ */
        /*     ret 0;\n\ */
       /* }", */
        /* "fn main\n\ */
        /* {\n\ */
        /*     fn plus (a: int, b: int)\n\ */
        /*     {\n\ */
        /*         fn get_a() { ret a; }\n\ */
        /*         fn get_b() { ret b; }\n\ */
        /*     }\n\ */
        /*     ret 0; \n\ */
        /* }" */
        /* "fn main\n\ */
        /*  {\n\ */
        /*      fn blah { 1; }\n\ */
        /*  }", */
        /* "fn main : string\n\ */
        /*  {\n\ */
        /*      ret 0;\n\ */
        /*  }", */
        /* "fn main\n\ */
        /*  {\n\ */
        /*      fn blah (a: int, b: int) { a; }\n\ */
        /*      blah (1, 2);\n\ */
        /*  }", */
        /* "fn main\n\ */
        /*  {\n\ */
        /*      \"jfdkfj\" + \"fksjdfks\";\n\ */
        /*     ret 0;\n\ */
        /*  }", */
        /* "fn main\n\ */
        /*  {\n\ */
        /*      a = 5;\n\ */
        /*  }", */
        /* "fn main\n\ */
        /*  {\n\ */
        /*      while (1) { 1 + 1; break }\n\ */
        /*      ret 0;\n\ */
        /*  }", */
        /* "fn main\n\ */
        /*  {\n\ */
        /*      while (1) { 1 + 1; }\n\ */
        /*      ret 0;\n\ */
        /*  }", */
        /* "fn main\n\ */
        /*  {\n\ */
        /*      if (1) { 1; } else { 2; }\n\ */
        /*      ret 0;\n\ */
        /*  }", */
        /* "fn main\n\ */
        /* {\n\ */
        /*     let point: Type;\n\ */
        /*     point;\n\ */
        /* }", */
        "fn main\n\
        {\n\
            let point: int = [ 1, \"blah\" ];\n\
            point;\n\
        }",
    };

    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

int main (void)
{
    const struct CMUnitTest tests[] =
    {
        /* cmocka_unit_test (macro__panic), */
        /* cmocka_unit_test (macro__assert), */
        cmocka_unit_test (main__return),
    };
    return cmocka_run_group_tests (tests, NULL, NULL);
}
