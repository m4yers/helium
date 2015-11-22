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
#include "escape.h"
#include "errormsg.h"

static void static_nesting_ok (void ** state)
{
    const char * source = 
    "let \
        var x:int := 10 \
        function zero (a:int, b:string) = \
            print (x) \
     in \
        nil \
     end";

    A_exp ast = parseString (source);

    assert_non_null (ast);

    Esc_FindEscape (ast);

    assert_true (ast->u.let.decs->head->u.var.escape);
    assert_false (ast->u.let.decs->tail->head->u.function->head->params->head->escape);

    (void) state;
}

int main (void)
{
    EM_disable ();
    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test (static_nesting_ok),
    };
    return cmocka_run_group_tests (tests, NULL, NULL);
}
