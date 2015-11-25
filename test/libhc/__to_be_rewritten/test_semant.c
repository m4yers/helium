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
#include "mipsmachine.h"
#include "program.h"

struct test_case
{
    const char * code;
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
        printf ("'%s': expected %s, got %s", test_case.code, test_case.ty->meta.name, actual->meta.name);
        fail();
    }
}

static void run_cases (void ** state, const struct test_case cases[], size_t len)
{
    for (size_t i = 0; i < len; ++i)
    {
        Program_Module m = Program_ModuleNew();

        if (Parse_String (m, cases[i].code) != 0)
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

        Program_PrintAssembly(stdout, m);

        /* assert_type (cases[i], r.ty); */
    }

    (void) state; /* unused */
}

static void decl_success (void ** state)
{
    struct test_case cases[] =
    {
        {"int main() { 1337; }", Ty_Int()},
    };

    run_cases (state, cases, TOTAL_ELEMENTS (cases));
}

int main (void)
{
    EM_disable();
    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test (decl_success),
    };
    return cmocka_run_group_tests (tests, NULL, NULL);
}
