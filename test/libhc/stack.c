#define TOTAL_ELEMENTS(array) (sizeof(array) / sizeof(array[0]))

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "ext/stack.h"

static void stack_creation_ok (void ** state)
{
    Stack s = Stack_New (sizeof (int), NULL);
    assert_non_null (s);
    assert_true (s->itemSize == sizeof (int));
    Stack_Delete (s);

    (void) state;
}

static void stack_storring_ok (void ** state)
{
    Stack s = Stack_New (sizeof (int), NULL);
    int number = 1000;
    for (int i = 0; i < number; ++i)
    {
        Stack_Push (s, &i);
    }
    assert_true (s->length == number);
    for (int i = number - 1; i >= 0; --i)
    {
        int sv;
        Stack_Pop (s, &sv);
        assert_true (sv == i);
    }
    (void) state;
}

static void stack_delete_semifull_ok (void ** state)
{
    (void) state;
}

int main (void)
{
    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test (stack_creation_ok),
        cmocka_unit_test (stack_storring_ok),
        cmocka_unit_test (stack_delete_semifull_ok),
    };
    return cmocka_run_group_tests (tests, NULL, NULL);
}
