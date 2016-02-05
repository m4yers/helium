#define TOTAL_ELEMENTS(array) (sizeof(array) / sizeof(array[0]))

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "util/list.h"
#include "util/vector.h"

#include "core/symbol.h"
#include "core/parse.h"
#include "core/canon.h"
#include "core/codegen.h"
#include "core/regalloc.h"
#include "core/error.h"
#include "core/program.h"

#include "modules/helium/env.h"
#include "modules/helium/preproc.h"
#include "modules/helium/translate.h"
#include "modules/helium/escape.h"
#include "modules/helium/semant.h"
#include "modules/mips/machine.h"

static void run_cases (void ** state, const char * cases[], size_t len)
{
    for (size_t i = 0; i < len; ++i)
    {
        Program_Module m = Program_ModuleNew();

        /* printf ("Program:\n%s\n", cases[i]); */

        if (Parse_String (m, cases[i]) != 0)
        {
            printf ("Lex errors: %lu\n", Vector_Size (&m->errors.lexer));
            VECTOR_FOREACH (struct Error_t, error, &m->errors.lexer)
            {
                Error_Print (stdout, error);
            }

            printf ("Parse errors: %lu\n", Vector_Size (&m->errors.parser));
            VECTOR_FOREACH (struct Error_t, error, &m->errors.parser)
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
            VECTOR_FOREACH (struct Error_t, error, &m->errors.preproc)
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
            VECTOR_FOREACH (struct Error_t, error, &m->errors.semant)
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

static void main__return (void ** state)
{
    const char * cases[] =
    {
        /* "fn main\n\ */
        /* {\n\ */
        /*     def Point = { x: int, y: int }\n\ */
        /*     let a = 10;\n\ */
        /*     let b: Point;\n\ */
        /*     asm\n\ */
        /*     {\n\ */
        /*          la    `t0, \"blahblah\"   \n\ */
        /*          lw    `t1, 0(`t0)         \n\ */
        /*          addi  `t0, `t0, 0x04      \n\ */
        /*          add   `t1, `t1, `t0       \n\ */
        /*          li    `t2, 0xffff0008     \n\ */
        /*          li    `t3, 0xffff000c     \n\ */
        /*      ``wait:                       \n\ */
        /*          lw    `t4, 0(`t2)         \n\ */
        /*          beq   `t4, $zero, ``wait  \n\ */
        /*          beq   `t0, `t1, ``exit    \n\ */
        /*          lbu   `t5, 0(`t0)         \n\ */
        /*          sb    `t5, 0(`t3)         \n\ */
        /*          addi  `t0, `t0, 0x01      \n\ */
        /*          j     ``wait              \n\ */
        /*      ``exit:                       \n\ */
        /*          addi  `t0, $zero, 0x0A    \n\ */
        /*          sb    `t0, 0(`t3)         \n\ */
        /*     }\n\ */
        /*     ret 1;\n\ */
        /* }\n\ */
        /* asm{addi $a0, $a1, 1111}", */

        /* "fn main\n\ */
        /* {\n\ */
        /*     let a = 10;\n\ */
        /* \n\ */
        /*     asm\n\ */
        /*     {\n\ */
        /*         addi a, $zero, 1337\n\ */
        /*     }\n\ */
        /* }", */

        "fn main\n\
        {\n\
            def Point = { x: int, y: int }\n\
            1;\n\
        }",

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
