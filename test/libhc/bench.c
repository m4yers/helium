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

        LIST_FOREACH (f, m->fragments.code)
        {
            T_stmList sl = C_Linearize (f->u.code.body);
            ASM_lineList lines = F_CodeGen (NULL, sl);
            Vector_PushBack(&m->results.code, lines);
        }

        LIST_FOREACH (f, m->fragments.functions)
        {
            T_stmList sl = C_Linearize (f->u.proc.body);
            sl = C_TraceSchedule (C_BasicBlocks (sl));
            ASM_lineList lines = F_CodeGen (f->u.proc.frame, sl);
            /* ASM_PrintLineList (stdout, lines, Temp_LayerMap (F_tempMap, Temp_Name ())); */
            F_frame frame = f->u.proc.frame;
            lines = F_ProcEntryExit2 (frame, lines);
            RA_Result rar = RA_RegAlloc (frame, lines, regs_all, regs_colors);
            lines = F_ProcEntryExit3 (frame, lines, rar->colors);

            Vector_PushBack (&m->results.functions, rar);
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
        "fn main\n\
        {\n\
            asm\n\
            {\n\
                addi $a0, $a1, 1337\n\
            }\n\
            \n\
            ret 1;\n\
        }\n\
        asm{addi $a0, $a1, 1111}",

        /* "fn main\n\ */
        /* {\n\ */
        /*     sum(1,2);\n\ */
        /*     ret 0;\n\ */
        /* }\n\ */
        /* asm(mips;;)\n\ */
        /* {\n\ */
        /*     sum:\n\ */
        /*       add   $v0, $a0, $a1\n\ */
        /*       jr    $ra\n\ */
        /* }", */

        /* "fn main\n\ */
        /* {\n\ */
        /*     def Point = { x: int, y: int }\n\ */
        /*     let a = Point{ x = 10, y = 11 };\n\ */
        /*     a = Point{ x = 10, y = 11 };\n\ */
        /*     ret 1;\n\ */
        /* }", */
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
