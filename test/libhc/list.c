#define TOTAL_ELEMENTS(array) (sizeof(array) / sizeof(array[0]))

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "ext/list.h"

LIST_DEFINE (ItemList, size_t)

static void list_push_ok (void ** state)
{
    ItemList list = NULL;
    size_t size = 10;
    for (size_t i = 0; i < size; ++i)
    {
        LIST_PUSH (list, i)
    }

    size_t i = 0;
    while (list)
    {
        assert_true (list->head == i);
        list = list->tail;
        i++;
    }

    assert_true (i == size);

    (void) state;
}

static void list_push_unique_ok (void ** state)
{
    ItemList list = NULL;
    size_t size = 10;
    for (size_t i = 0; i < size; ++i)
    {
        LIST_PUSH_UNIQUE (list, i)
        LIST_PUSH_UNIQUE (list, i)
        LIST_PUSH_UNIQUE (list, i)
    }

    size_t i = 0;
    while (list)
    {
        assert_true (list->head == i);
        list = list->tail;
        i++;
    }

    assert_true (i == size);

    (void) state;
}

static void list_size_ok (void ** state)
{
    ItemList list = NULL;
    size_t size = 10;
    for (size_t i = 0; i < size; ++i)
    {
        LIST_PUSH (list, i)
    }

    assert_true (LIST_SIZE (list) == size);

    (void) state;
}

static void list_back_ok (void ** state)
{
    ItemList list = NULL;
    size_t size = 10;
    for (size_t i = 0; i < size; ++i)
    {
        LIST_PUSH (list, i)
    }

    assert_true (LIST_BACK (list) == (size - 1));

    (void) state;
}

static void list_at_ok (void ** state)
{
    ItemList list = NULL;
    size_t size = 10;
    for (size_t i = 0; i < size; ++i)
    {
        LIST_PUSH (list, i)
    }

    for (size_t i = 0; i < size; ++i)
    {
        assert_true (LIST_AT (list, i) == i);
    }

    (void) state;
}

static void list_remove_ok (void ** state)
{
    ItemList list = NULL;
    size_t size = 10;
    for (size_t i = 0; i < size; ++i)
    {
        LIST_PUSH (list, i)
    }

    assert_true (LIST_SIZE (list) == size);

    LIST_REMOVE (list, 9)
    LIST_REMOVE (list, 5)
    LIST_REMOVE (list, 9)

    assert_true (LIST_SIZE (list) == size - 2);

    (void) state;
}

static void list_insert_ok (void ** state)
{
    size_t size = 5;

    ItemList list = NULL;

    LIST_INSERT (list, 3, 0)
    LIST_INSERT (list, 0, 0)
    LIST_INSERT (list, 1, 1)
    LIST_INSERT (list, 4, 1000)
    LIST_INSERT (list, 2, 2)

    for (size_t i = 0; i < size; ++i)
    {
        assert_true (LIST_AT (list, i) == i);
    }

    (void) state;
}

static void list_join_ok (void ** state)
{
    size_t size = 10;

    ItemList l1 = NULL;
    for (size_t i = 0; i < size / 2; ++i)
    {
        LIST_PUSH (l1, i)
    }

    ItemList l2 = NULL;
    for (size_t i = size / 2; i < size; ++i)
    {
        LIST_PUSH (l2, i)
    }

    LIST_JOIN (l1, l2)

    for (size_t i = 0; i < size; ++i)
    {
        assert_true (LIST_AT (l1, i) == i);
    }

    (void) state;
}

static void list_inject_ok (void ** state)
{
    size_t size = 10;

    ItemList l1 = NULL;
    LIST_PUSH (l1, 0)
    LIST_PUSH (l1, 1)
    LIST_PUSH (l1, 5)
    LIST_PUSH (l1, 6)
    LIST_PUSH (l1, 7)
    LIST_PUSH (l1, 8)
    LIST_PUSH (l1, 9)

    ItemList l2 = NULL;
    LIST_PUSH (l2, 2)
    LIST_PUSH (l2, 3)
    LIST_PUSH (l2, 4)

    LIST_INJECT (l1, l2, 2)

    for (size_t i = 0; i < size; ++i)
    {
        assert_true (LIST_AT (l1, i) == i);
    }

    (void) state;
}

static void list_foreach_ok (void ** state)
{
    ItemList list = NULL;
    size_t size = 10;
    for (size_t i = 0; i < size; ++i)
    {
        LIST_PUSH (list, i)
    }

    size_t n = 0;
    LIST_FOREACH (i, list)
    {
        assert_true (i == n++);
    }

    (void) state;
}

int main (void)
{
    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test (list_push_ok),
        cmocka_unit_test (list_push_unique_ok),
        cmocka_unit_test (list_size_ok),
        cmocka_unit_test (list_back_ok),
        cmocka_unit_test (list_at_ok),
        cmocka_unit_test (list_remove_ok),
        cmocka_unit_test (list_insert_ok),
        cmocka_unit_test (list_join_ok),
        cmocka_unit_test (list_inject_ok),
        cmocka_unit_test (list_foreach_ok),
    };
    return cmocka_run_group_tests (tests, NULL, NULL);
}
