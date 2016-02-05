#define TOTAL_ELEMENTS(array) (sizeof(array) / sizeof(array[0]))

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "util/table.h"

static void table_foreach_ok (void ** state)
{
    const char * strings[] =
    {
        "one",
        "two",
        "three",
        "four",
        "five"
    };

    TAB_table table = TAB_Empty();

    // should not enter the block
    TAB_FOREACH(k, v, table)
    {
        fail();
    }

    for (size_t i = 0; i < TOTAL_ELEMENTS(strings); ++i)
    {
        TAB_Enter(table, strings[i], strings[i]);
    }

    size_t i = TOTAL_ELEMENTS(strings);
    TAB_FOREACH(k, v, table)
    {
        i--;
        assert_true(strings[i] == k);
        assert_true(strings[i] == v);
        /* printf("%s:%s\n", k, v); */
    }

    (void) state;
}

int main (void)
{
    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test (table_foreach_ok),
    };
    return cmocka_run_group_tests (tests, NULL, NULL);
}
