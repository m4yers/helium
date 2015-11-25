#define TOTAL_ELEMENTS(array) (sizeof(array) / sizeof(array[0]))

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "ext/bitarray.h"

static void array_creation_ok (void ** state)
{
    BitArray bits;

    bits = BitArray_New (32, FALSE);
    assert_true (* (uint32_t *)bits->blob == 0);

    bits = BitArray_New (32, TRUE);
    assert_true (* (uint32_t *)bits->blob == UINT32_MAX);

    (void) state;
}

static void array_set_unset_ok (void ** state)
{
    BitArray bits;

    bits = BitArray_New (32, FALSE);
    for (int i = 0; i < bits->length; ++i)
    {
        assert_false (BitArray_IsSet (bits, i));
    }

    bits = BitArray_New (32, TRUE);
    for (int i = 0; i < bits->length; ++i)
    {
        assert_true (BitArray_IsSet (bits, i));
    }

    // Has set
    bits = BitArray_New (32, FALSE);
    BitArray_Set (bits, 5);
    assert_true (BitArray_HasSet (bits));

    //Degree
    bits = BitArray_New (32, FALSE);
    BitArray_Set (bits, 1);
    BitArray_Set (bits, 5);
    BitArray_Set (bits, 7);
    BitArray_Set (bits, 11);
    BitArray_Set (bits, 31);
    assert_true (BitArray_GetDegree (bits) == 5);

    (void) state;
}

static void array_logical_ok (void ** state)
{
    BitArray a, b, r;

    // Or
    a = BitArray_New (32, FALSE);
    b = BitArray_New (32, TRUE);
    r = BitArray_Or (a, b, NULL);
    assert_true (* (uint32_t *)r->blob == UINT32_MAX);

    a = BitArray_New (32, FALSE);
    b = BitArray_New (32, TRUE);
    r = BitArray_Or (a, b, a);
    assert_true (a == r);
    assert_true (* (uint32_t *)r->blob == UINT32_MAX);

    // And
    a = BitArray_New (32, FALSE);
    b = BitArray_New (32, TRUE);
    r = BitArray_And (a, b, NULL);
    assert_true (* (uint32_t *)r->blob == 0);

    a = BitArray_New (32, FALSE);
    b = BitArray_New (32, TRUE);
    r = BitArray_And (a, b, a);
    assert_true (a == r);
    assert_true (* (uint32_t *)r->blob == 0);

    // Not
    a = BitArray_New (32, FALSE);
    r = BitArray_Not (a, NULL);
    assert_true (* (uint32_t *)r->blob == UINT32_MAX);

    a = BitArray_New (32, FALSE);
    r = BitArray_Not (a, a);
    assert_true (a == r);
    assert_true (* (uint32_t *)r->blob == UINT32_MAX);

    (void) state;
}

static void array_sets_ok (void ** state)
{
    /*
     * Union and Intersection essentially the same as Or and And,
     * and tested here
     */

    BitArray a, b, r;

    a = BitArray_New (32, FALSE);
    a->blob[0] = 255;
    a->blob[1] = 255;
    b = BitArray_New (32, FALSE);
    b->blob[1] = 255;
    b->blob[2] = 255;
    r = BitArray_RelDifference (a, b, NULL);
    assert_true (* (uint32_t *)r->blob == UINT8_MAX);

    a = BitArray_New (32, FALSE);
    a->blob[0] = 255;
    a->blob[1] = 255;
    b = BitArray_New (32, FALSE);
    b->blob[1] = 255;
    b->blob[2] = 255;
    r = BitArray_RelDifference (a, b, a);
    assert_true (a == r);
    assert_true (* (uint32_t *)r->blob == UINT8_MAX);

    (void) state;
}

static void matrix_creation_ok (void ** state)
{
    BitMatrix m;

    m = BitMatrix_New (10);
    assert_non_null (m);
    assert_true (m->width = 10);
    assert_true (m->allocated = BitArray_CalculateSize (10 * 10));

    (void) state;
}

static void matrix_row_collumn_ok (void ** state)
{
    BitMatrix m;
    BitArray a;

    // Live row object
    m = BitMatrix_New (10);
    a = BitMatrix_GetRow (m, 3);
    BitArray_Set (a, 3);
    assert_true (BitMatrix_IsSet (m, 3, 3));

    BitMatrix_Set (m, 3, 5);
    assert_true (BitArray_IsSet (a, 5));

    BitMatrix_UnSet (m, 3, 5);
    assert_false (BitArray_IsSet (a, 5));

    // Copy row object
    m = BitMatrix_New (10);
    a = BitMatrix_GetRowCopy (m, 3, NULL);
    BitArray_Set (a, 3);
    assert_false (BitMatrix_IsSet (m, 3, 3));

    BitMatrix_Set (m, 3, 5);
    assert_false (BitArray_IsSet (a, 5));

    // we get the row again and this time the setting will be there
    BitArray r = BitMatrix_GetRowCopy (m, 3, a);
    assert_true (BitArray_IsSet (r, 5));
    assert_true (r == a);

    // Copy collumn object
    m = BitMatrix_New (18);
    BitMatrix_Set (m, 1, 3);
    BitMatrix_Set (m, 2, 3);
    BitMatrix_Set (m, 13, 3);
    BitMatrix_Set (m, 6, 3);
    BitMatrix_Set (m, 17, 3);
    a = BitArray_New (18, FALSE);
    BitArray_Set (a, 1);
    BitArray_Set (a, 2);
    BitArray_Set (a, 13);
    BitArray_Set (a, 6);
    BitArray_Set (a, 17);
    assert_true (BitArray_Equal (a, BitMatrix_GetCollumnCopy (m, 3, NULL)));

    // Unset collumn
    m = BitMatrix_New (10);
    BitMatrix_Set (m, 1, 3);
    BitMatrix_Set (m, 5, 3);
    BitMatrix_Set (m, 7, 3);
    BitMatrix_UnSetColumn (m, 3);
    assert_true (BitMatrix_Equal (m, BitMatrix_New (10)));

    // Unset zeroth column
    m = BitMatrix_New (10);
    BitMatrix_Set (m, 1, 0);
    BitMatrix_Set (m, 5, 0);
    BitMatrix_Set (m, 7, 0);
    BitMatrix_UnSetColumn (m, 0);
    assert_true (BitMatrix_Equal (m, BitMatrix_New (10)));

    // Set column
    a = BitArray_New (10, FALSE);
    BitArray_Set (a, 0);
    BitArray_Set (a, 5);
    BitArray_Set (a, 7);
    m = BitMatrix_New (10);
    BitMatrix_SetColumnFromArray (m, a, 3);
    BitMatrix n = BitMatrix_New (10);
    BitMatrix_Set (n, 0, 3);
    BitMatrix_Set (n, 5, 3);
    BitMatrix_Set (n, 7, 3);
    assert_true (BitMatrix_Equal (m, n));

    //Degree
    m = BitMatrix_New (10);
    BitMatrix_Set (m, 1, 1);
    BitMatrix_Set (m, 1, 4);
    BitMatrix_Set (m, 1, 8);
    assert_true (BitMatrix_GetRowDegree (m, 1) == 3);

    (void) state;
}

int main (void)
{
    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test (array_creation_ok),
        cmocka_unit_test (array_logical_ok),
        cmocka_unit_test (array_sets_ok),
        cmocka_unit_test (array_set_unset_ok),
        cmocka_unit_test (matrix_creation_ok),
        cmocka_unit_test (matrix_row_collumn_ok),
    };
    return cmocka_run_group_tests (tests, NULL, NULL);
}
