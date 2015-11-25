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

typedef struct test_case
{
    const char * code;
    const char * asem;
} test_case;

static void run_cases (void ** state, const struct test_case cases[], size_t len)
{
    for (size_t i = 0; i < len; ++i)
    {
        A_exp ast = parseString (cases[i].code);
        assert_non_null (ast);

        Esc_FindEscape (ast);

        E_Init();
        Tr_Init();

        SEM_Context context;
        context.level = Tr_Global();
        context.tenv = E_base_tenv();
        context.venv = E_base_venv();
        context.loopNesting = 0;

        transExp (&context, ast);
        F_fragList fl = Tr_GetResult();

        for (; fl; fl = fl->tail)
        {
            if (fl->head->kind == F_procFrag)
            {
                T_stmList sl = C_Linearize (fl->head->u.proc.body);
                sl = C_TraceSchedule (C_BasicBlocks (sl));
                ASM_lineList lines = F_CodeGen (fl->head->u.proc.frame, sl);
                /* ASM_PrintLineList (stdout, lines, Temp_LayerMap (F_tempMap, Temp_Name ())); */
                F_frame frame = fl->head->u.proc.frame;
                lines = F_ProcEntryExit2 (lines);
                struct RA_result colored = RA_RegAlloc (frame, lines, regs_all, regs_colors);
                lines = F_ProcEntryExit3 (frame, lines, colored.colors);
                ASM_PrintLineList (stdout, lines, Temp_LayerMap (colored.coloring, Temp_Name()));
            }
            else if (fl->head->kind == F_stringFrag)
            {
                printf ("%s: %s\n", fl->head->u.str.label->name, fl->head->u.str.str);
            }
        }
    }

    (void) state; /* unused */
}

static void multiply_ok (void ** state)
{
    struct test_case cases[] =
    {
        {
            "let \
             function main ():int = \
                  let\
                      var a:int := 1 * 5 \
                  in \
                     a \
                  end \
             in \
                 nil \
             end",
            ""
        },
    };
    run_cases (state, cases, TOTAL_ELEMENTS (cases));

}

static void division_ok (void ** state)
{
    struct test_case cases[] =
    {
        {
            "let \
             function main ():int = \
                  let\
                      var a:int := 1 / 5 \
                  in \
                     a \
                  end \
             in \
                 nil \
             end",
            ""
        },
    };
    run_cases (state, cases, TOTAL_ELEMENTS (cases));

}

static void if_stm_ok (void ** state)
{
    struct test_case cases[] =
    {
        {
            "let \
             function main ():int = \
                if 6 > 4 then 4 + 6\
             in \
                 nil \
             end",
            ""
        },
    };
    run_cases (state, cases, TOTAL_ELEMENTS (cases));

}

static void while_stm_ok (void ** state)
{
    struct test_case cases[] =
    {
        {
            "let \
             function main ():int = \
                while 6 > 4 do 2 + 3\
             in \
                 nil \
             end",
            ""
        },
    };
    run_cases (state, cases, TOTAL_ELEMENTS (cases));

}

static void var_ints_ok (void ** state)
{
    struct test_case cases[] =
    {
        {
            "let \
             function main ():int = \
                  let\
                      var a:int := 1 + 2 \
                      var b:int := 3 + 5 \
                  in \
                     a + b \
                  end \
             in \
                 1 \
             end",
            ""
        },
    };
    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

static void ints_vars_ok (void ** state)
{
    struct test_case cases[] =
    {
        {
            "let \
             function main ():int = \
                  let\
                      var a := 1337 \
                  in \
                      a := 1111; \
                      a + 123123 \
                  end \
             in \
                 1 \
             end",
            ""
        },
    };
    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

static void var_int_arrays_ok (void ** state)
{
    struct test_case cases[] =
    {
        {
            "let \
             function main ():int = \
                  let\
                      type int_array = array of int \
                      var  table := int_array[100] of 1337 \
                  in \
                      table[10] \
                  end \
             in \
                 1 \
             end",
            ""
        },
    };
    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

static void ints_record_ok (void ** state)
{
    struct test_case cases[] =
    {
        {
            "let \
             function main ():int = \
                  let\
                      type rec = { a:int, b:int, c:int } \
                      var blah := rec{ a = 1337, b = 1234, c = 9876 } \
                  in \
                      blah.a := 1111 \
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
             function long (a:int, b:int, c:int, d:int, e:int):int = \
                a + b + c \
             function short (a:int, b:int):int = \
                a + b \
             function one ():int = \
                1 \
             function main ():int = \
                long(5, 6, 7, 8, 9) \
             in \
                 nil \
             end",
            ""
        },
    };
    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

static void string_vars_ok (void ** state)
{
    struct test_case cases[] =
    {
        {
            "let \
             function main ():string = \
                  let\
                      var a := \"a cool string\" \
                      var b := \"a blah string\" \
                  in \
                      a \
                  end \
             in \
                 1 \
             end",
            ""
        },
    };
    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

static void string_arrays_ok (void ** state)
{
    struct test_case cases[] =
    {
        {
            "let \
             function main ():string = \
                  let\
                      type int_array = array of string \
                      var  table := int_array[100] of \"blah\" \
                  in \
                      table[45] \
                  end \
             in \
                 1 \
             end",
            ""
        },
    };
    run_cases (state, cases, TOTAL_ELEMENTS (cases));
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

int main (void)
{
    const struct CMUnitTest tests[] =
    {
        /* cmocka_unit_test (multiply_ok), */
        /* cmocka_unit_test (division_ok), */
        /* cmocka_unit_test (if_stm_ok), */
        /* cmocka_unit_test (while_stm_ok), */
        /* cmocka_unit_test (var_ints_ok), */
        /* cmocka_unit_test (var_int_arrays_ok), */
        /* cmocka_unit_test (ints_vars_ok), */
        /* cmocka_unit_test (ints_record_ok), */
        cmocka_unit_test (function_calls_ok),
        /* cmocka_unit_test (string_vars_ok), */
        /* cmocka_unit_test (string_arrays_ok), */
        /* cmocka_unit_test (string_records_ok), */
    };
    return cmocka_run_group_tests (tests, NULL, NULL);
}
