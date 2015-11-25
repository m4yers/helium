#define TOTAL_ELEMENTS(array) (sizeof(array) / sizeof(array[0]))

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "parse.h"
#include "escape.h"

static void static_nesting_ok (void ** state)
{
    const char * code =
    "fn main\
    {\
        let x = 10;\
        fn func (a: int) { print (x); }\
    }";

    Program_Module m = Program_ModuleNew();
    Parse_String (m, code);

    assert_non_null (m->ast);

    Escape_Find (m->ast);

    assert_true (m->ast->head->u.function.scope->list->head->u.dec->u.var.escape);
    assert_false (m->ast->head->u.function.scope->list->tail->head->u.dec->u.function.params->head->escape);

    (void) state;
}

int main (void)
{
    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test (static_nesting_ok),
    };
    return cmocka_run_group_tests (tests, NULL, NULL);
}
