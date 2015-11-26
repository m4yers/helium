#define TOTAL_ELEMENTS(array) (sizeof(array) / sizeof(array[0]))

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "ext/str.h"

static const char * a_string = "This is a string";

static void string_on_stack_ok (void ** state)
{
    struct String_t str = String (a_string);

    assert_true (str.data == a_string);
    assert_true (str.size == strlen (a_string));
    assert_true (str.capacity == strlen (a_string) + 1);
    assert_true (str.is_static);

    String_Fini (&str);

    assert_null (str.data);
    assert_true (str.size == 0);
    assert_true (str.capacity == 0);

    (void) state;
}

static void string_on_heap_ok (void ** state)
{
    String str = String_New (a_string);

    assert_true (str->data == a_string);
    assert_true (str->size == strlen (a_string));
    assert_true (str->capacity == strlen (a_string) + 1);
    assert_true (str->is_static);

    String_Delete (str);

    assert_null (str);

    (void) state;
}

static void string_static_ok (void ** state)
{
    String str = String_New (a_string);

    assert_true (String_IsStatic (str));
    assert_true (String_Data (str) == a_string);
    String_NoStatic (str);
    assert_false (String_IsStatic (str));
    assert_false (String_Data (str) == a_string);

    (void) state;
}

static void string_empty_ok (void ** state)
{
    struct String_t str = String ("");

    assert_true (String_Empty (&str));

    (void) state;
}

static void string_clear_ok (void ** state)
{
    String str;

    str = String_New (a_string);
    assert_false (String_Empty (str));
    String_Clear (str);
    assert_true (String_Empty (str));

    str = String_New (a_string);
    String_NoStatic (str);
    assert_false (String_Empty (str));
    String_Clear (str);
    assert_true (String_Empty (str));

    (void) state;
}

static void string_reserve_ok (void ** state)
{
    String str = String_New (a_string);

    assert_true (String_IsStatic (str));
    assert_true (String_Data (str) == a_string);

    String_Reserve (str, 100);

    assert_false (String_IsStatic (str));
    assert_true (String_Capacity (str) == 100);
    assert_true (String_Size (str) == strlen (a_string));
    assert_false (String_Data (str) == a_string);
    assert_true (strcmp (a_string, String_Data (str)) == 0);

    (void) state;
}

static void string_push_back_ok (void ** state)
{
    String str = String_New (a_string);

    assert_true (String_IsStatic (str));
    assert_true (String_Data (str) == a_string);

    String_PushBack (str, '\n');

    assert_false (String_IsStatic (str));
    assert_false (String_Data (str) == a_string);
    assert_true (String_Size (str) == (strlen (a_string) + 1));
    assert_false (strcmp (a_string, String_Data (str)) == 0);

    (void) state;
}

/* static void string__ok (void ** state) */
/* { */
/*     (void) state; */
/* } */

int main (void)
{
    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test (string_on_stack_ok),
        cmocka_unit_test (string_on_heap_ok),
        cmocka_unit_test (string_static_ok),
        cmocka_unit_test (string_empty_ok),
        cmocka_unit_test (string_clear_ok),
        cmocka_unit_test (string_reserve_ok),
        cmocka_unit_test (string_push_back_ok),
    };
    return cmocka_run_group_tests (tests, NULL, NULL);
}
