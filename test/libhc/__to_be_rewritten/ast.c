#define TOTAL_ELEMENTS(array) (sizeof(array) / sizeof(array[0]))

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "symbol.h"
#include "parse.h"
#include "env.h"
#include "semant.h"
#include "translate.h"

static void run_cases (void ** state, const char ** cases, size_t len)
{
    for (size_t i = 0; i < len; ++i)
    {
        Program_Module m = Program_ModuleNew();

        if (Parse_String (m, cases[i]) != 0)
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

        assert_true (m->ast);

        AST_Print (stdout, m->ast, 0);
    }

    (void) state; /* unused */
}

static void control__if (void ** state)
{
    const char * cases[] =
    {
        "fn main\n\
        {\n\
            if (a) { b; }\n\
        }",
        "fn main\n\
        {\n\
            if (a) { b; } else { c; }\n\
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

static void decl__functions (void ** state)
{
    const char * cases[] =
    {
        "fn main { 1337; }",
        "fn main { 1337; }",
        "fn main : int { 1337; }",
        "fn main\
        {\
            let x: int;\
            let y: int = 1337;\
            let z = 1337;\
        }",
        "fn main\
        {\
            let x = 3 + 5;\
            let y = x;\
            \
            fn mul(a: int, b: int)\
            {\
                a * b;\
            }\
            \
            let c = mul(x,y);\
        }",
        "fn main\
        {\
            let a = [1,2,3,4,5];\
            a[0] = { x = 5, y = 5 }; \
            println(\"x: %d, y: %d\", a[0].x, a[0].y); \
        }",
        "fn main \
         { \
             let a = sub (1337, 313); \
             fn add(a: int, b: int) { a + b; } \
             add(a, 10); \
         }",
    };

    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

static void decl__types(void ** state)
{
    const char * cases[] =
    {
        /* "def Point = { x: int, y: int }\ */
        /*  let a = {x = 3, y = 10};", */
        "def Point = { x: int, y: int }\
         fn main\
         {\
             let a = Point { x = 10, y = 10 }; \
             a.x; \
         }",
    };

    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

static void stack__arrays(void ** state)
{
    const char * cases[] =
    {
        "let a: [int;10];",
        "let a = [ 1, 2, 3 ];",
        "fn main : [int;10]\
        {\
            let a: [int;10]; \
        }",
    };

    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

static void stack__records(void ** state)
{
    const char * cases[] =
    {
        "let point: {x: int, y: int};",
        "let point = {x=5,y=5};",
        "let point: {x: int, y: int} = { x = 5, y = 3 };",
        "fn main : {x: int, y: int}\
        {\
            { x = 5, y = 3 }; \
        }",
    };

    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

static void stack__records_nested(void ** state)
{
    const char * cases[] =
    {
        "let point: {x: {a: int, b: int}, y: int};",
    };

    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

int main (void)
{
    const struct CMUnitTest tests[] =
    {
        /* cmocka_unit_test (control__if), */
        /* cmocka_unit_test (control__while), */
        cmocka_unit_test (control__for),
        /* cmocka_unit_test (decl__functions), */
        /* cmocka_unit_test (decl__types), */
        /* cmocka_unit_test (stack__arrays), */
        /* cmocka_unit_test (stack__records), */
        /* cmocka_unit_test (stack__records_nested), */
    };
    return cmocka_run_group_tests (tests, NULL, NULL);
}
