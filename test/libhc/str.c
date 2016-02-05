#define TOTAL_ELEMENTS(array) (sizeof(array) / sizeof(array[0]))

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <assert.h>

#include "util/str.h"
#include "util/vector.h"

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

    assert_true (String_Diff (String_New (""), "") == -1);
    assert_true (String_Diff (String_New ("a"), "") == 0);
    assert_true (String_Diff (String_New (""), "a") == 0);

    assert_true (String_Diff (String_New ("aaaa"), "yyyy") == 0);
    assert_true (String_Diff (String_New ("aaaa"), "ayyy") == 1);
    assert_true (String_Diff (String_New ("aaaa"), "aayy") == 2);
    assert_true (String_Diff (String_New ("aaaa"), "aaay") == 3);

    assert_true (String_Diff (String_New ("aaaa"), "aaaa") == -1);

    assert_true (String_Diff (String_New ("yyyy"), "aaaa") == 0);
    assert_true (String_Diff (String_New ("ayyy"), "aaaa") == 1);
    assert_true (String_Diff (String_New ("aayy"), "aaaa") == 2);
    assert_true (String_Diff (String_New ("aaay"), "aaaa") == 3);

    assert_true (String_Diff (String_New ("aa"), "yyyy") == 0);
    assert_true (String_Diff (String_New ("aa"), "ayyy") == 1);
    assert_true (String_Diff (String_New ("aa"), "aayy") == 2);
    assert_true (String_Diff (String_New ("aa"), "aaay") == 2);
    assert_true (String_Diff (String_New ("aa"), "aaaa") == 2);

    assert_true (String_Diff (String_New ("yyyy"), "aa") == 0);
    assert_true (String_Diff (String_New ("ayyy"), "aa") == 1);
    assert_true (String_Diff (String_New ("aayy"), "aa") == 2);
    assert_true (String_Diff (String_New ("aaay"), "aa") == 2);
    assert_true (String_Diff (String_New ("aaaa"), "aa") == 2);

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

static void string_append_ok (void ** state)
{
    String str = String_New (a_string);
    const char * append = "blah";

    assert_true (String_IsStatic (str));
    assert_true (String_Data (str) == a_string);

    String_Append (str, append);

    assert_false (String_IsStatic (str));
    assert_false (String_Data (str) == a_string);

    /*
     * a_string: xxxxxxxxx0
     *      str: xxxxxxxxxyyyy0
     *                   ^
     */
    assert_true (String_Size (str) == (strlen (a_string) + strlen (append)));
    assert_true (String_Diff (String_New (a_string), str) == (int)strlen (a_string));

    (void) state;
}

static void string_append_integer_ok (void ** state)
{
    String str = String_New ("13");

    assert_true (String_IsStatic (str));
    assert_true (String_Equal (str, "13"));

    String_Append (str, 37);

    assert_false (String_IsStatic (str));
    assert_true (String_Equal (str, "1337"));

    (void) state;
}

/**
 * appending a zero length string basically does nothing
 */
static void string_append_zero_len_ok (void ** state)
{
    String str = String_New (a_string);
    const char * append = "";

    assert_true (String_IsStatic (str));
    assert_true (String_Data (str) == a_string);

    String_Append (str, append);

    assert_true (String_IsStatic (str));
    assert_true (String_Data (str) == a_string);

    assert_true (String_Size (str) == (strlen (a_string)));
    assert_true (String_Diff (String_New (a_string), str) == -1);

    (void) state;
}

static void string_appendf_ok (void ** state)
{
    String str = String_New ("");
    String_Reserve (str, 512);

    String_AppendF (str, "%d", 100);

    assert_true (String_Equal (str, "100"));
    assert_true (String_Size (str) == 3);

    String_AppendF (str, "%d", 500);

    assert_true (String_Equal (str, "100500"));
    assert_true (String_Size (str) == 6);

    (void) state;
}

static void string_assign_ok (void ** state)
{
    String str = String_New (a_string);
    const char * assign = "blah";

    assert_true (String_IsStatic (str));
    assert_true (String_Data (str) == a_string);

    String_Assign (str, assign);

    assert_false (String_IsStatic (str));
    assert_false (String_Data (str) == a_string);
    assert_false (String_Data (str) == assign);

    assert_true (String_Size (str) == (strlen (assign)));
    assert_true (String_Diff (String_New (assign), str) == -1);

    (void) state;
}

static void string_assign_empty_ok (void ** state)
{
    String str = String_New (a_string);
    const char * assign = "";

    assert_true (String_IsStatic (str));
    assert_true (String_Data (str) == a_string);

    String_Assign (str, assign);

    assert_false (String_IsStatic (str));
    assert_false (String_Data (str) == a_string);
    assert_false (String_Data (str) == assign);

    assert_true (String_Size (str) == (strlen (assign)));
    assert_true (String_Diff (String_New (assign), str) == -1);

    (void) state;
}

static void string_assign_uninitialized_ok (void ** state)
{
    struct String_t str;
    const char * assign = "blah";

    String_Assign (&str, assign);

    assert_false (String_IsStatic (&str));
    assert_false (String_Data (&str) == a_string);
    assert_false (String_Data (&str) == assign);

    assert_true (String_Size (&str) == (strlen (assign)));
    assert_true (String_Diff (String_New (assign), &str) == -1);

    (void) state;
}

static void string_insert_front_ok (void ** state)
{
    const char * first = "axxxxxxb";
    const char * second = "yyy";
    const char * combined = "yyyaxxxxxxb";

    String str = String_New (first);

    String_Insert (str, 0, second);

    assert_true (String_Size (str) == strlen (combined));
    assert_true (String_Equal (String_New (combined), str));

    (void) state;
}

static void string_insert_middle_ok (void ** state)
{
    const char * first = "axxxxxxb";
    const char * second = "yyy";
    const char * combined = "axxyyyxxxxb";

    String str = String_New (first);

    String_Insert (str, 3, second);

    assert_true (String_Size (str) == strlen (combined));
    assert_true (String_Equal (String_New (combined), str));

    (void) state;
}

static void string_insert_back_ok (void ** state)
{
    const char * first = "axxxxxxb";
    const char * second = "yyy";
    const char * combined = "axxxxxxbyyy";

    String str = String_New (first);

    String_Insert (str, strlen (first), second);

    assert_true (String_Size (str) == strlen (combined));
    assert_true (String_Equal (String_New (combined), str));

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

static void string_split_ok (void ** state)
{
    String str = NULL;
    Vector split = NULL;
    size_t c = 0;

    /** empty string */
    str = String_New ("");
    split = String_Split (str, ',');

    assert_true (Vector_Size (split) == 0);

    /** many seps */
    str = String_New (",,,");
    split = String_Split (str, ',');

    assert_true (Vector_Size (split) == 4);

    VECTOR_FOREACH (struct String_t, s, split)
    {
        assert_true (String_Equal (s, ""));
    }

    /** 3 items separated by , */
    const char * strs[] = {"one", "two", "three"};
    str = String_New ("one,two,three");
    split = String_Split (str, ',');

    assert_true (Vector_Size (split) == 3);

    c = 0;
    VECTOR_FOREACH (struct String_t, s, split)
    {
        assert_true (String_Equal (s, strs[c++]));
    }

    /** 3 items separated by sep and one preceding sep */
    const char * strs4[] = {"", "one", "two", "three"};
    str = String_New (",one,two,three");
    split = String_Split (str, ',');

    assert_true (Vector_Size (split) == 4);

    c = 0;
    VECTOR_FOREACH (struct String_t, s, split)
    {
        assert_true (String_Equal (s, strs4[c++]));
    }

    /** 3 items separated by sep and one following sep */
    const char * strs5[] = {"one", "two", "three", ""};
    str = String_New ("one,two,three,");
    split = String_Split (str, ',');

    assert_true (Vector_Size (split) == 4);

    c = 0;
    VECTOR_FOREACH (struct String_t, s, split)
    {
        assert_true (String_Equal (s, strs5[c++]));
    }

    /** one item */
    str = String_New ("one");
    split = String_Split (str, ',');

    assert_true (Vector_Size (split) == 1);

    VECTOR_FOREACH (struct String_t, s, split)
    {
        assert_true (String_Equal (s, "one"));
    }

    /** one item with separator at the front */
    const char * strs2[] = {"", "one"};
    str = String_New (",one");
    split = String_Split (str, ',');

    assert_true (Vector_Size (split) == 2);

    c = 0;
    VECTOR_FOREACH (struct String_t, s, split)
    {
        assert_true (String_Equal (s, strs2[c++]));
    }

    /** one item with separator at the back */
    const char * strs3[] = {"one", ""};
    str = String_New ("one,");
    split = String_Split (str, ',');

    assert_true (Vector_Size (split) == 2);

    c = 0;
    VECTOR_FOREACH (struct String_t, s, split)
    {
        assert_true (String_Equal (s, strs3[c++]));
    }

    (void) state;
}

static void string_foreach_ok (void ** state)
{
    String str = String_New (a_string);

    size_t offset = 0;
    STRING_FOREACH (c, str)
    {
        assert_true (c == a_string[offset++]);
    }

    assert_true (offset == strlen (a_string));

    offset = 0;
    STRING_FOREACH_PTR (c, str)
    {
        assert_true (*c == a_string[offset++]);
    }

    assert_true (offset == strlen (a_string));

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
        cmocka_unit_test (string_append_ok),
        cmocka_unit_test (string_append_integer_ok),
        cmocka_unit_test (string_append_zero_len_ok),
        cmocka_unit_test (string_appendf_ok),
        cmocka_unit_test (string_assign_ok),
        cmocka_unit_test (string_assign_empty_ok),
        cmocka_unit_test (string_assign_uninitialized_ok),
        cmocka_unit_test (string_insert_front_ok),
        cmocka_unit_test (string_insert_middle_ok),
        cmocka_unit_test (string_insert_back_ok),
        cmocka_unit_test (string_front_ok),
        cmocka_unit_test (string_back_ok),
        cmocka_unit_test (string_at_ok),
        cmocka_unit_test (string_split_ok),
        cmocka_unit_test (string_foreach_ok),
    };
    return cmocka_run_group_tests (tests, NULL, NULL);
}
