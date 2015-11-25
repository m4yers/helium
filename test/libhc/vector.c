#define TOTAL_ELEMENTS(array) (sizeof(array) / sizeof(array[0]))

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "ext/vector.h"

static void vector_ints_ok (void ** state)
{
    int count = 10;

    Vector v = Vector_New (int);

    for (int i = 0; i < count; ++i)
    {
        Vector_PushBack (v, i);
    }

    assert_true (Vector_Size (v) == (size_t) count);

    for (int i = 0; i < count; ++i)
    {
        assert_true (* (int *)Vector_At (v, i) == i);
    }

    int index = 0;
    VECTOR_FOREACH (int, item, v)
    {
        assert_true (*item == index++);
    }

    Vector_Delete (v);

    (void) state;
}

static void vector_strings_ok (void ** state)
{
    int count = 10;
    const char * strings[] = { "0th", "1st", "2nd", "3rd", "4th", "5th", "6th", "7th", "8th", "9th", };

    Vector v = Vector_New (const char *);

    for (int i = 0; i < count; ++i)
    {
        Vector_PushBack (v, strings[i]);
    }

    assert_true (Vector_Size (v) == (size_t) count);

    for (int i = 0; i < count; ++i)
    {
        assert_true (* (const char **)Vector_At (v, i) == strings[i]);
    }

    int index = 0;
    VECTOR_FOREACH (const char *, item, v)
    {
        assert_true (*item == strings[index++]);
    }

    Vector_Delete (v);

    (void) state;
}

static void vector_structs_ok (void ** state)
{
    int count = 10;

    struct a_type
    {
        const char * str;
        int value;
    };

    struct a_type structs[] =
    {
        {"0th", 0 },
        {"1st", 1 },
        {"2nd", 2 },
        {"3rd", 3 },
        {"4th", 4 },
        {"5th", 5 },
        {"6th", 6 },
        {"7th", 7 },
        {"8th", 8 },
        {"9th", 9 },
    };

    Vector v = Vector_New (struct a_type);

    for (int i = 0; i < count; ++i)
    {
        Vector_PushBack (v, structs[i]);
    }

    assert_true (Vector_Size (v) == (size_t) count);

    for (int i = 0; i < count; ++i)
    {
        struct a_type * a = Vector_At (v, i);
        struct a_type * b = &structs[i];

        // these are two different structs, vector stores full copy
        assert_false (a == b);

        // but pointers point to the same object
        assert_true (a->str == b->str);

        // primitive types
        assert_true (a->value == b->value);
    }

    int index = 0;
    VECTOR_FOREACH (struct a_type, item, v)
    {
        struct a_type * a = item;
        struct a_type * b = &structs[index++];

        // these are two different structs, vector stores full copy
        assert_false (a == b);

        // but pointers point to the same object
        assert_true (a->str == b->str);

        // primitive types
        assert_true (a->value == b->value);
    }

    Vector_Delete (v);

    (void) state;
}

int asserts;
static void assert_true_func (void * flag)
{
    assert_true (* (bool *)flag);
    asserts++;
}

static void del(void * blah)
{
    (void) blah;
    Vector_Delete(*(Vector*)blah);
}

static void vector_of_vectors_ok (void ** state)
{
    int count = 10;

    Vector v = Vector_New (Vector);
    Vector_SetDestructor (v, del);

    for (int i = 0; i < count; ++i)
    {
        Vector a = NULL;
        switch (i % 3)
        {
        case 0:
        {
            a = Vector_New (int);
            for (int j = 0; j < count; ++j)
            {
                Vector_PushBack (a, j);
            }
            break;
        }
        case 1:
        {
            a = Vector_New (double);
            for (int j = 0; j < count; ++j)
            {
                Vector_PushBack (a, (double)j);
            }
            break;
        }
        case 2:
        {
            a = Vector_New (bool);
            Vector_SetDestructor (a, assert_true_func);
            for (int j = 0; j < count; ++j)
            {
                Vector_PushBack (a, TRUE);
            }
            break;
        }
        }

        Vector_PushBack (v, a);
    }

    asserts = 0;
    Vector_Delete (v);

    assert_true (asserts == count * (count / 3));

    (void) state;
}

int main (void)
{
    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test (vector_ints_ok),
        cmocka_unit_test (vector_strings_ok),
        cmocka_unit_test (vector_structs_ok),
        cmocka_unit_test (vector_of_vectors_ok),
    };
    return cmocka_run_group_tests (tests, NULL, NULL);
}
