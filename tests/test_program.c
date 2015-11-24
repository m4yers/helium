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
            printf ("Lex errors: %lu\n", Vector_Size (m->errors.lexer));
            VECTOR_FOREACH (struct Error, error, m->errors.lexer)
            {
                Error_Print (stdout, error);
            }

            printf ("Parse errors: %lu\n", Vector_Size (m->errors.parser));
            VECTOR_FOREACH (struct Error, error, m->errors.parser)
            {
                Error_Print (stdout, error);
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
            VECTOR_FOREACH (struct Error, error, m->errors.semant)
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

            Vector_PushBack (m->results, rar);

        }

        Program_PrintAssembly (stdout, m);
    }

    (void) state; /* unused */
}

static void control__if (void ** state)
{
    const char * cases[] =
    {
        /* "fn main\n\ */
        /*  {\n\ */
        /*      if (1) { 2; }\n\ */
        /*  }", */
        "fn main\n\
         {\n\
             let a = 3;\
             if (a) { 2; } else { 3; }\n\
         }",
    };

    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

static void control__while (void ** state)
{
    const char * cases[] =
    {
        "fn main\n\
        {\n\
            while (1) { 1 + 1; }\n\
        }",
    };

    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

static void control__for (void ** state)
{
    const char * cases[] =
    {
        "fn main\n\
        {\n\
            for (i = 0 to 100 ) { 1 + 1; }\n\
        }",
    };

    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

static void string_records_ok (void ** state)
{
    const char * cases[] =
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
    };
    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

static void decl__functions (void ** state)
{
    const char * cases[] =
    {
        "fn sub (a: int, b: int) { a - b; } \
         fn main \
         { \
             let a = sub (1337, 313); \
             fn add(a: int, b: int) { a + b; } \
             add(a, 10); \
         }",
    };
    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

static void decl__functions_nested (void ** state)
{
    const char * cases[] =
    {
        "fn main\n\
        {\n\
            let leet = 1337;\n\
            fn doit\n\
            {\n\
                fn sum (a: int, b: int) { a + b + leet; }\n\
                sum (3, 5);\n\
            }\n\
            doit();\n\
        }",
    };
    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

static void decl__types (void ** state)
{
    const char * cases[] =
    {
        /* "def Point = { x: int, y: int }\ */
        /*  fn main\ */
        /*  {\ */
        /*      let a: Point; \ */
        /*      a.x = 10; \ */
        /*      a.y = 10;\ */
        /*      a.x; \ */
        /*  }", */
        /* "def Point = { x: int, y: int }\ */
        /*  fn main\ */
        /*  {\ */
        /*      let a = Point { x = 10, y = 10 }; \ */
        /*      a.x; \ */
        /*  }", */
        /* "def Point = { x: int, y: int }\ */
        /*  fn main\ */
        /*  {\ */
        /*      let a = Point { y = 10 }; \ */
        /*      a.x; \ */
        /*  }", */
        /* "def Point = { x: int, y: int }\ */
        /*  fn main\ */
        /*  {\ */
        /*      let a: Point = Point { y = 10 }; \ */
        /*      a.x; \ */
        /*  }", */
        "def Point = { x: int, y: int }\
         fn main\
         {\
             let a = Point{ y = 10, x = 1337 }; \
             a.x; \
         }",
    };
    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

static void stack__arrays (void ** state)
{
    const char * cases[] =
    {
        "fn main\n\
        {\n\
            let a: [int;5];\n\
            a[0] = 1000;\n\
            a[4] = 1004;\n\
            a[4];\n\
        }",
        "fn main\n\
        {\n\
            let a = [ 1, 2, 3, 4, 5 ];\n\
            a[4];\n\
        }",
    };
    run_cases (state, cases, TOTAL_ELEMENTS (cases));

}

static void stack__arrays_nested (void ** state)
{
    const char * cases[] =
    {
        /* "fn main\n\ */
        /* {\n\ */
        /*     def Point = { x: int, y: int }\n\ */
        /*     let a = [ Point{ y = 10 }, Point{ x = 20 } ];\n\ */
        /*     a[0].x;\n\ */
        /* }", */
        "fn main\n\
        {\n\
            def Point = { x: int, y: int }\n\
            def Arr = [Point;5]\n\
            def Bla = { x: Arr, y: int }\n\
            let a = Bla{ y = 10 };\n\
        }",
    };
    run_cases (state, cases, TOTAL_ELEMENTS (cases));

}

static void stack__records (void ** state)
{
    const char * cases[] =
    {
        /* "fn main()\ */
        /* {\ */
        /*     let point: { x: int, y: int }; \ */
        /*     point.x = 3; \ */
        /*     point.y = 5; \ */
        /*     point.y; \ */
        /* }", */
        "fn main()\
        {\
            let point = { x = 3, y = 5 }; \
            point.y; \
        }",
    };
    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

static void stack__records_nested (void ** state)
{
    const char * cases[] =
    {
        /* "fn main\ */
        /*  {\ */
        /*      let point: {x: {a: int, b: int}, y: {a :int, b: {foo: int,bar: int}}};\ */
        /*      point.y.b.bar;\ */
        /*  }", */
        /* "fn main\ */
        /*  {\ */
        /*      let point: { x: [int;5], y: [int;10] };\ */
        /*      point.x[0] + point.y[0];\ */
        /*  }", */
        /* "fn main\ */
        /*  {\ */
        /*      let point = { x = { a = 2, b = 3 }, y = { c = 4, d = { q = 5, w = 6 } } };\ */
        /*      point.y.d.q;\ */
        /*  }", */
        /* "fn main\ */
        /*  {\ */
        /*      let point = { i = 8, x = [ 1, 2, 3, 4, 5], y = 10 };\ */
        /*      point.x[3];\ */
        /* }", */
        /* "fn main\n\ */
        /*  {\n\ */
        /*      def Point = { x: [int;3], y: int }\n\ */
        /*      let rect = { tl = Point { y = 5 }, br = Point { y = 10 } };\n\ */
        /*  }", */
        /* "fn main\n\ */
        /*  {\n\ */
        /*      def Point = { x: { a: int, b: int }, y: int }\n\ */
        /*      let rect = {\ */
        /*          tl = Point{\ */
        /*              x = { a = 33, b = 44 },\ */
        /*              y = 3 }\ */
        /*      };\n\ */
        /*      rect.tl.x.a;\ */
        /*  }", */
        /* "fn main\n\ */
        /*  {\n\ */
        /*      def Point = { x: { a: int, b: int }, y: int }\n\ */
        /*      let rect = { tl = Point{}, br = Point{} };\n\ */
        /*      rect.tl.x.a;\ */
        /*  }", */
        "fn main\n\
         {\n\
             def Point = { x: int, y: int }\n\
             def Rect  = { tl: Point, br: Point }\n\
             let str = \"Blah\";\n\
             let rect = Rect{};\n\
             rect.tl.x;\
         }",
    };

    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

int main (void)
{
    const struct CMUnitTest tests[] =
    {
        /* cmocka_unit_test (control__if), */
        /* cmocka_unit_test (control__while), */
        /* cmocka_unit_test (control__for), */
        /* cmocka_unit_test (string_records_ok), */
        /* cmocka_unit_test (decl__functions), */
        /* cmocka_unit_test (decl__functions_nested), */
        /* cmocka_unit_test (decl__types), */
        /* cmocka_unit_test (stack__arrays), */
        /* cmocka_unit_test (stack__arrays_nested), */
        /* cmocka_unit_test (stack__records), */
        cmocka_unit_test (stack__records_nested),
    };
    return cmocka_run_group_tests (tests, NULL, NULL);
}
