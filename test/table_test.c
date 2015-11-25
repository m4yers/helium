#define TOTAL_ELEMENTS(array) (sizeof(array) / sizeof(array[0]))

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "table.h"

static void table_getKeys_ok (void ** state)
{
    TAB_table table = TAB_Empty();

    (void) state;
}

int main (void)
{
    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test (table_getKeys_ok),
    };
    return cmocka_run_group_tests (tests, NULL, NULL);
}
