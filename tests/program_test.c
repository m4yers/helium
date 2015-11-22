#define TOTAL_ELEMENTS(array) (sizeof(array) / sizeof(array[0]))

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "prabsyn.h"
#include "symbol.h"
#include "parse.h"
#include "env.h"
#include "semant.h"
#include "translate.h"
#include "canon.h"
#include "errormsg.h"
#include "codegen.h"
#include "regalloc.h"
#include "mipsmachine.h"
#include "escape.h"

#include "program.h"

typedef struct test_case
{
    const char * code;
    const char * asem;
} test_case;

static void run_cases (void ** state, const struct test_case cases[], size_t len)
{
    for (size_t i = 0; i < len; ++i)
    {
        Program_Module m = Program_ModuleNew();

        if (Parse_String (m, cases[0].code) != 0)
        {
            printf ("Lex errors: %lu\n", Vector_Size (m->errors.lexer));
            VECTOR_FOREACH (const char *, error, m->errors.lexer)
            {
                printf (" - %s\n", *error);
            }

            printf ("Parse errors: %lu\n", Vector_Size (m->errors.parser));
            VECTOR_FOREACH (const char *, error, m->errors.parser)
            {
                printf (" - %s\n", *error);
            }

            fail();
        }

        MIPS_Init();
        F_Init();

        assert_true (m->ast);

        if (Semant_Translate (m) != 0)
        {
            printf ("Failed to translate program\n");
            printf ("Semant errors: %lu\n", Vector_Size (m->errors.semant));
            VECTOR_FOREACH (const char *, error, m->errors.semant)
            {
                printf (" - %s\n", *error);
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
            lines = F_ProcEntryExit2 (lines);
            RA_Result rar = RA_RegAlloc (frame, lines, regs_all, regs_colors);
            lines = F_ProcEntryExit3 (frame, lines, rar->colors);

            Vector_PushBack (m->results, rar);

        }

        Program_PrintAssembly (stdout, m);
    }

    (void) state; /* unused */
}

static void string_records_ok (void ** state)
{
    struct test_case cases[] =
    {
        {
            "let \
             function main ():string = \
                  let\
                      type rec = { a:string, b: int } \
                      var blah := rec{ a = \"blah\", b = 1337 } \
                  in \
                      blah.a \
                  end \
             in \
                 1 \
             end",
            ""
        },
    };
    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

static void function_calls_ok (void ** state)
{
    struct test_case cases[] =
    {
        {
            "let \
                function main ():int = \
                    let \
                        var blah := 1337 \
                        function sum (a:int, b:int):int = \
                            a + b \
                    in \
                        sum (blah, 5) \
                    end \
            in \
                 nil \
            end",
            ""
        },
    };
    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

static void nested_function_calls_ok (void ** state)
{
    struct test_case cases[] =
    {
        {
            "let\
                function main ():int =\
                    let\
                        // level x\
                        var blah := 1337\
                        function do():int = \
                            let\
                                // level x + 1\
                                function sum (a:int, b:int):int =\
                                    a + b + blah\
                            in\
                                sum (3, 5)\
                            end\
                    in\
                        do ()\
                    end\
            in\
                 nil\
            end",
            ""
        },
    };
    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

int main (void)
{
    const struct CMUnitTest tests[] =
    {
        /* cmocka_unit_test (string_records_ok), */
        /* cmocka_unit_test (function_calls_ok), */
        cmocka_unit_test (nested_function_calls_ok),
    };
    return cmocka_run_group_tests (tests, NULL, NULL);
}
