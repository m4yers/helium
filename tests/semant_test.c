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
#include "errormsg.h"

struct test_case
{
    const char * exp;
    Ty_ty ty;
};

/**
 *
 * Shallow type checking. Does not check the addresses of custom types
 */
static bool types_are_equal (Ty_ty left, Ty_ty right)
{
    //printf ("%s vs %s\n", left->meta.name, right->meta.name);
    if (left->kind != right->kind)
    {
        return FALSE;
    }

    switch (left->kind)
    {
    case Ty_array:
    {
        return types_are_equal (left->u.array, right->u.array);
    }
    case Ty_record:
    {
        for (Ty_fieldList l = left->u.record, r = right->u.record; l || r; l = l->tail, r = r->tail)
        {
            if (!l || !r || !types_are_equal (l->head->ty, r->head->ty))
            {
                return FALSE;
            }
        }
        return TRUE;
    }
    default:; // silence -Wswitch
    }

    return TRUE;
}

static void assert_type (struct test_case test_case, Ty_ty actual)
{
    if (!types_are_equal (test_case.ty, actual))
    {
        fail_msg ("'%s': expected %s, got %s", test_case.exp, test_case.ty->meta.name, actual->meta.name);
    }
}

static void run_cases (void ** state, const struct test_case cases[], size_t len)
{
    for (size_t i = 0; i < len; ++i)
    {
        A_exp ast = parseString (cases[i].exp);
        assert_non_null (ast);

        E_Init();
        Tr_Init ();

        SEM_Context context;
        context.level = Tr_Global();
        context.tenv = E_base_tenv();
        context.venv = E_base_venv();
        context.loopNesting = 0;

        expty r = transExp (&context, ast);
        assert_type (cases[i], r.ty);
    }

    (void) state; /* unused */
}

static void decl_success (void ** state)
{
    struct test_case cases[] =
    {
        {"let var a:int := 1 in a end", Ty_Int()},
        {"let var a := 1 in a end", Ty_Int()},
        {"let type a = int in nil end", Ty_Nil()},

        {
            "let \
            type a = array of int \
            var b:a := a[100] of 0 \
          in \
            b \
          end", Ty_Array (Ty_Int())
        },

        {
            "let \
            type a = array of int \
            var b := a[100] of 0 \
          in \
            b \
          end", Ty_Array (Ty_Int())
        },

        {
            "let \
            type a = { a:int, b:string } \
          in \
            nil \
          end", Ty_Nil()
        },

        {
            "let \
            type x = { z:int }\
            type a = { a:x, b:string } \
            var b:a := a { a = x{ z = 1 }, b = \"string\"} \
          in \
            b.a.z \
          end", Ty_Int()
        },

        {
            "let \
            type a = { a:int, b:string } \
            var b:a := a { a = 1, b = \"string\"} \
          in \
            b \
          end",

            Ty_Record (Ty_FieldList (
                Ty_Field (S_Symbol ("a"), Ty_Int()),
                Ty_FieldList (
                    Ty_Field (S_Symbol ("b"), Ty_String()),
                    NULL)))
        },

        {
            "let \
            function zero (a:int, b:string):int = 0 \
          in \
              zero () \
          end", Ty_Int()
        },
    };

    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

static void recursive_decs_ok (void ** state)
{
    struct test_case cases[] =
    {
        {
            "let \
            type a = { a:int, b:b } \
            type b = { a:int, c:string } \
          in \
            nil \
          end", Ty_Nil()
        },

        {
            "let \
            function zero (a:int, b:string):int = blah() \
            function blah (a:int, b:string):int = \
                let \
                    function foo (a:int, b:string):int = blah() \
                in \
                    nil \
                end \
          in \
              zero () \
          end", Ty_Int()
        },
    };
    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

static void contorls_ok (void ** state)
{
    struct test_case cases[] =
    {
        { "while 1 do nil", Ty_Void()},
        { "if 1 then 1", Ty_Void()},
        { "if 1 then 1 else 1", Ty_Int()},
        { "if 1 then 1 else \"string\"", Ty_Void()},
        {"for id := 1 to 5 do nil", Ty_Void()},
    };
    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

static void nil_ok (void ** state)
{
    struct test_case cases[] =
    {
        { "nil", Ty_Nil() }
    };
    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

static void ints_ok (void ** state)
{
    const struct test_case cases[] =
    {
        { "-1", Ty_Int() },
    };
    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

static void strings_ok (void ** state)
{
    struct test_case cases[] =
    {
        { "\"ya stroka\"", Ty_String() }
    };
    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

static void operators_ok (void ** state)
{
    const struct test_case cases[] =
    {
        { "-\"a string\"", Ty_Invalid() },
        { "\"a string\" + \"another string\"", Ty_Invalid() },
        { "\"a string\" - \"another string\"", Ty_Invalid() },
        { "\"a string\" * \"another string\"", Ty_Invalid() },
        { "\"a string\" / \"another string\"", Ty_Invalid() },
        { "\"a string\" = \"another string\"", Ty_Int() },
        { "\"a string\" > \"another string\"", Ty_Invalid() },
        { "\"a string\" < \"another string\"", Ty_Invalid() },
        { "\"a string\" <> \"another string\"", Ty_Int() },
        { "\"a string\" >= \"another string\"", Ty_Invalid() },
        { "\"a string\" <= \"another string\"", Ty_Invalid() },
        { "\"a string\" + 1", Ty_Invalid() },
        { "\"a string\" - 1", Ty_Invalid() },
        { "\"a string\" * 1", Ty_Invalid() },
        { "\"a string\" / 1", Ty_Invalid() },
        { "\"a string\" = 1", Ty_Invalid() },
        { "\"a string\" > 1", Ty_Invalid() },
        { "\"a string\" < 1", Ty_Invalid() },
        { "\"a string\" <> 1", Ty_Invalid() },
        { "\"a string\" >= 1", Ty_Invalid() },
        { "\"a string\" <= 1", Ty_Invalid() },
        { "-1", Ty_Int() },
        { "1 + 1", Ty_Int() },
        { "1 - 1", Ty_Int() },
        { "1 * 1", Ty_Int() },
        { "1 / 1", Ty_Int() },
        { "1 > 1", Ty_Int() },
        { "1 < 1", Ty_Int() },
        { "1 = 1", Ty_Int() },
        { "1 <> 1", Ty_Int() },
        { "1 >= 1", Ty_Int() },
        { "1 <= 1", Ty_Int() },
        { "1 <= 1", Ty_Int() },
        { "1 * 1 + 1 / 1 - 1 * 1", Ty_Int() },
        { "1 * \"string\" * 5 / 3", Ty_Invalid() }
    };
    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

int main (void)
{
    EM_disable();
    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test (decl_success),
        cmocka_unit_test (recursive_decs_ok),
        cmocka_unit_test (nil_ok),
        cmocka_unit_test (ints_ok),
        cmocka_unit_test (strings_ok),
        cmocka_unit_test (operators_ok),
        cmocka_unit_test (contorls_ok),
    };
    return cmocka_run_group_tests (tests, NULL, NULL);
}
