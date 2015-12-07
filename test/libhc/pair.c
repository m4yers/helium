#define TOTAL_ELEMENTS(array) (sizeof(array) / sizeof(array[0]))

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "ext/pair.h"

static const char * a_string = "This is a string";

static void pair_on_stack_ok (void ** state)
{
    struct Pair_t pair = { .first = NULL, .second = NULL };

    Pair_Init (&pair, 1337, "blah");

    assert_true (* (int *)pair.first == 1337);
    assert_true (strcmp (* (char **)pair.second, "blah") == 0);

    assert_true (* (int *)Pair_First (&pair) == 1337);
    assert_true (strcmp (* (char **)Pair_Second (&pair), "blah") == 0);

    (void) state;
}

static void pair_on_heap_ok (void ** state)
{
    /* Pair pair = Pair_New (1337, "blah"); */
    /*  */
    /* Pair_Init (pair, 1337, "blah"); */
    /*  */
    /* assert_true (* (int *)pair->first == 1337); */
    /* assert_true (strcmp (* (char **)pair->second, "blah") == 0); */
    /*  */
    /* assert_true (* (int *)Pair_First (pair) == 1337); */
    /* assert_true (strcmp (* (char **)Pair_Second (pair), "blah") == 0); */
    /*  */
    /* (void) state; */
}
int main (void)
{
    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test (pair_on_stack_ok),
        cmocka_unit_test (pair_on_heap_ok),
    };
    return cmocka_run_group_tests (tests, NULL, NULL);
}
