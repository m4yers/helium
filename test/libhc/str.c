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

static void string_no_static_ok (void ** state)
{
    String str = String_New (a_string);

    assert_true (str->is_static);

    String_NoStatic (str);

    assert_false (str->is_static);
    assert_true (strcmp (str->data, a_string) == 0);

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

static void string_diff_ok (void ** state)
{
    assert_true (String_Diff (String_New (""), String_New ("")) == -1);
    assert_true (String_Diff (String_New ("a"), String_New ("")) == 0);
    assert_true (String_Diff (String_New (""), String_New ("a")) == 0);

    assert_true (String_Diff (String_New ("aaaa"), String_New ("yyyy")) == 0);
    assert_true (String_Diff (String_New ("aaaa"), String_New ("ayyy")) == 1);
    assert_true (String_Diff (String_New ("aaaa"), String_New ("aayy")) == 2);
    assert_true (String_Diff (String_New ("aaaa"), String_New ("aaay")) == 3);

    assert_true (String_Diff (String_New ("aaaa"), String_New ("aaaa")) == -1);

    assert_true (String_Diff (String_New ("yyyy"), String_New ("aaaa")) == 0);
    assert_true (String_Diff (String_New ("ayyy"), String_New ("aaaa")) == 1);
    assert_true (String_Diff (String_New ("aayy"), String_New ("aaaa")) == 2);
    assert_true (String_Diff (String_New ("aaay"), String_New ("aaaa")) == 3);

    assert_true (String_Diff (String_New ("aa"), String_New ("yyyy")) == 0);
    assert_true (String_Diff (String_New ("aa"), String_New ("ayyy")) == 1);
    assert_true (String_Diff (String_New ("aa"), String_New ("aayy")) == 2);
    assert_true (String_Diff (String_New ("aa"), String_New ("aaay")) == 2);
    assert_true (String_Diff (String_New ("aa"), String_New ("aaaa")) == 2);

    assert_true (String_Diff (String_New ("yyyy"), String_New ("aa")) == 0);
    assert_true (String_Diff (String_New ("ayyy"), String_New ("aa")) == 1);
    assert_true (String_Diff (String_New ("aayy"), String_New ("aa")) == 2);
    assert_true (String_Diff (String_New ("aaay"), String_New ("aa")) == 2);
    assert_true (String_Diff (String_New ("aaaa"), String_New ("aa")) == 2);

    String str = String_New (a_string);
    String_NoStatic (str);
    assert_true (String_Diff (str, String_New (a_string)) == -1);

    (void) state;
}

static void string_cmp_ok (void ** state)
{
    assert_true (String_Cmp (String_New ("aaaa"), String_New ("aaaa")) == 0);
    assert_true (String_Cmp (String_New ("aa"), String_New ("aaaa")) != 0);
    assert_true (String_Cmp (String_New ("aaaa"), String_New ("aa")) != 0);

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

    /*
     * a_string: xxxxxxxxx0
     *      str: xxxxxxxxxy0
     *                    ^
     */
    assert_true (String_Size (str) == (strlen (a_string) + 1));
    assert_true (String_Diff (String_New (a_string), str) == (int)strlen (a_string));

    (void) state;
}

static void string_pop_back_ok (void ** state)
{
    String str = String_New (a_string);

    assert_true (String_IsStatic (str));
    assert_true (String_Data (str) == a_string);

    String_PopBack (str);

    assert_false (String_IsStatic (str));
    assert_false (String_Data (str) == a_string);

    /*
     * a_string: xxxxxxxxx0
     *      str: xxxxxxxx0
     *                   ^
     */
    assert_true (String_Size (str) == (strlen (a_string) - 1));
    assert_true (String_Diff (String_New (a_string), str) == (int)String_Size (str));

    (void) state;
}

static void string_front_ok (void ** state)
{
    String str = String_New (a_string);

    assert_true (String_Front (str) == a_string);

    String_NoStatic (str);

    assert_false (String_Front (str) == a_string);
    assert_true (String_Equal (String_New (a_string), str));

    (void) state;
}

static void string_back_ok (void ** state)
{
    String str = String_New (a_string);

    assert_true (String_Back (str) == a_string + strlen (a_string) - 1);

    String_NoStatic (str);

    assert_false (String_Back (str) == a_string + strlen (a_string) - 1);
    assert_true (String_Equal (String_New (a_string), str));

    (void) state;
}

static void string_at_ok (void ** state)
{
    String str = String_New (a_string);

    for (int i = 0, l = strlen (a_string); i < l; ++i)
    {
        assert_true (String_At (str, i) == a_string + i);
        assert_true (*String_At (str, i) == a_string[i]);
    }

    String_NoStatic (str);

    for (int i = 0, l = strlen (a_string); i < l; ++i)
    {
        assert_true (String_At (str, i) != a_string + i);
        assert_true (*String_At (str, i) == a_string[i]);
    }

    (void) state;
}

int main (void)
{
    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test (string_on_stack_ok),
        cmocka_unit_test (string_on_heap_ok),
        cmocka_unit_test (string_static_ok),
        cmocka_unit_test (string_empty_ok),
        cmocka_unit_test (string_clear_ok),
        cmocka_unit_test (string_no_static_ok),
        cmocka_unit_test (string_reserve_ok),
        cmocka_unit_test (string_diff_ok),
        cmocka_unit_test (string_cmp_ok),
        cmocka_unit_test (string_push_back_ok),
        cmocka_unit_test (string_pop_back_ok),
        cmocka_unit_test (string_front_ok),
        cmocka_unit_test (string_back_ok),
        cmocka_unit_test (string_at_ok),
    };
    return cmocka_run_group_tests (tests, NULL, NULL);
}
