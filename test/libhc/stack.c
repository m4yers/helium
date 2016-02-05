#define TOTAL_ELEMENTS(array) (sizeof(array) / sizeof(array[0]))

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "util/stack.h"

static void stack_creation_ok (void ** state)
{
    Stack s = Stack_New (size_t, NULL);
    assert_non_null (s);
    assert_true (s->itemSize == sizeof (size_t));
    Stack_Delete (s);

    (void) state;
}

static void stack_storring_ok (void ** state)
{
    Stack s = Stack_New (size_t, NULL);
    size_t number = 1000;

    for (size_t i = 0; i < number; ++i)
    {
        Stack_Push (s, i);
    }

    assert_true (Stack_Size (s) == number);

    while (number-- > 0)
    {
        size_t * sv = Stack_Top (s);
        assert_true (*sv == number);
        Stack_Pop (s);
    }

    (void) state;
}

static void stack_foreach_backwards_ok (void ** state)
{
    struct Stack_t s = Stack (size_t, NULL);

    size_t number = 1000;

    for (size_t i = 0; i < number; ++i)
    {
        Stack_Push (&s, i);
    }

    STACK_FOREACH_BACKWARDS (size_t, i, &s)
    {
        assert_true (*i == --number);
    }

    (void) state;
}

static void stack_foreach_ok (void ** state)
{
    struct Stack_t s = Stack (size_t, NULL);

    size_t number = 1000;

    for (size_t i = 0; i < number; ++i)
    {
        Stack_Push (&s, i);
    }

    size_t c = 0;
    STACK_FOREACH (size_t, i, &s)
    {
        assert_true (*i == c++);
    }

    (void) state;
}

int main (void)
{
    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test (stack_creation_ok),
        cmocka_unit_test (stack_storring_ok),
        cmocka_unit_test (stack_foreach_backwards_ok),
        cmocka_unit_test (stack_foreach_ok),
    };
    return cmocka_run_group_tests (tests, NULL, NULL);
}
