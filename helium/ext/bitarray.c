#define BITS_IN_BYTE 8

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "bitarray.h"
#include "mem.h"
#include "bool.h"

static const char * ByteToString (uint8_t x, char * buffer);

/*********************************************************************
 *                             BitArray                              *
 *********************************************************************/

size_t BitArray_CalculateSize (int bits)
{
    return bits / 8 + (bits % 8 ? 1 : 0);
}

BitArray BitArray_New (int length, bool set)
{
    assert (length > 0);

    BitArray r = checked_malloc (sizeof (*r));
    r->length = length;
    r->allocated = BitArray_CalculateSize (length);
    r->blob = checked_malloc (r->allocated);

    memset (r->blob, 0, r->allocated);

    if (set)
    {
        BitArray_SetAll (r);
    }

    return r;
}

void BitArray_Delete (BitArray bits)
{
    assert (bits);
    // We do not want to free memory used by someone else;)
    if (!bits->isView)
    {
        free (bits->blob);
    }
    free (bits);
}

bool BitArray_Equal (BitArray a, BitArray b)
{
    assert (a);
    assert (b);

    int offset = a->allocated;
    while (offset--)
    {
        if (a->blob[offset] != b->blob[offset])
        {
            return FALSE;
        }
    }

    return TRUE;

}

BitArray BitArray_Clone (BitArray a, BitArray r)
{
    assert (a);

    if (r)
    {
        assert (r->length == a->length);
    }
    else
    {
        r = BitArray_New (a->length, FALSE);
    }

    r->length = a->length;
    r->allocated = a->allocated;
    memcpy (r->blob, a->blob, a->allocated);

    return r;
}

char * BitArray_ToString (BitArray bits)
{
    assert (bits);

    char * r = checked_malloc (bits->length + bits->allocated);
    for (int i = 0; i < bits->allocated; ++i)
    {
        int offset = i * BITS_IN_BYTE + i;
        ByteToString (bits->blob[i], &r[offset]);
        r[offset + BITS_IN_BYTE] = ' ';
    }
    r[bits->length + bits->allocated - 1] = '\0';

    return r;
}

void BitArray_Print (FILE * out, BitArray bits)
{
    fprintf (out, "%s\n", BitArray_ToString (bits));
}

int BitArray_GetDegree (BitArray bits)
{
    int r = 0;
    int index = bits->length;
    while (index--)
    {
        int offset = index / BITS_IN_BYTE;
        int shift = index % BITS_IN_BYTE;
        if ((bits->blob[offset] >> shift) & 0x01)
        {
            r++;
        }
    }

    return r;
}

bool BitArray_HasSet (BitArray bits)
{
    assert (bits);

    int offset = bits->allocated;

    while (offset--)
    {
        if (bits->blob[offset] != 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}

int BitArray_FirstSet (BitArray bits)
{
    assert (bits);

    int index = bits->length;
    while (index--)
    {
        int offset = index / BITS_IN_BYTE;
        int shift = index % BITS_IN_BYTE;
        if ((bits->blob[offset] >> shift) & 0x01)
        {
            return index;
        }
    }

    return -1;
}

bool BitArray_IsSet (BitArray bits, int index)
{
    assert (bits);
    assert (index < bits->length);

    int offset = index / BITS_IN_BYTE;
    int shift = index % BITS_IN_BYTE;
    return (bits->blob[offset] >> shift) & 0x01;
}

void BitArray_Set (BitArray bits, int index)
{
    assert (bits);
    assert (index < bits->length);

    int offset = index / BITS_IN_BYTE;
    uint8_t mask = 0x01 << (index % BITS_IN_BYTE);
    bits->blob[offset] |= mask;
}

void BitArray_SetAll (BitArray bits)
{
    assert (bits);

    int index = bits->length;
    while (index--)
    {
        int offset = index / BITS_IN_BYTE;
        uint8_t mask = 0x01 << (index % BITS_IN_BYTE);
        bits->blob[offset] |= mask;
    }
}

void BitArray_UnSet (BitArray bits, int index)
{
    assert (bits);
    assert (index < bits->length);

    int offset = index / BITS_IN_BYTE;
    uint8_t mask = 0x01 << (index % BITS_IN_BYTE);
    bits->blob[offset] &= ~mask;
}

void BitArray_UnSetAll (BitArray bits)
{
    assert (bits);

    int index = bits->length;
    while (index--)
    {
        int offset = index / BITS_IN_BYTE;
        uint8_t mask = 0x01 << (index % BITS_IN_BYTE);
        bits->blob[offset] &= ~mask;
    }
}

BitArray BitArray_Or (BitArray a, BitArray b, BitArray r)
{
    assert (a);
    assert (b);

    /**
     * Normally it would not cause any troubles and the result set
     * would have been of MAX(a, b) length, but since we are dealing
     * with bit sets they MUST be of the same length or there is an
     * error in your logic.
     */
    assert (a->length == b->length);

    if (r)
    {
        assert (r->length == a->length);
    }
    else
    {
        r = BitArray_New (a->length, FALSE);
    }

    int offset = a->allocated;
    while (offset--)
    {
        r->blob[offset] = a->blob[offset] | b->blob[offset];
    }

    return r;
}

BitArray BitArray_And (BitArray a, BitArray b, BitArray r)
{
    assert (a);
    assert (b);

    /**
     * Normally it would not cause any troubles and the result set
     * would have been of MAX(a, b) length, but since we are dealing
     * with bit sets they MUST be of the same length or there is an
     * error in your logic.
     */
    assert (a->length == b->length);

    if (r)
    {
        assert (r->length == a->length);
    }
    else
    {
        r = BitArray_New (a->length, FALSE);
    }

    int offset = a->allocated;
    while (offset--)
    {
        r->blob[offset] = a->blob[offset] & b->blob[offset];
    }

    return r;
}

BitArray BitArray_Xor (BitArray a, BitArray b, BitArray r)
{
    assert (a);
    assert (b);

    /**
     * Normally it would not cause any troubles and the result set
     * would have been of MAX(a, b) length, but since we are dealing
     * with bit sets they MUST be of the same length or there is an
     * error in your logic.
     */
    assert (a->length == b->length);

    if (r)
    {
        assert (r->length == a->length);
    }
    else
    {
        r = BitArray_New (a->length, FALSE);
    }

    int offset = a->allocated;
    while (offset--)
    {
        r->blob[offset] = a->blob[offset] ^ b->blob[offset];
    }

    return r;
}

BitArray BitArray_Not (BitArray a, BitArray r)
{
    assert (a);

    if (r)
    {
        assert (r->length == a->length);
    }
    else
    {
        r = BitArray_New (a->length, FALSE);
    }

    int offset = a->allocated;
    while (offset--)
    {
        r->blob[offset] = ~a->blob[offset];
    }

    return r;
}

BitArray BitArray_Union (BitArray a, BitArray b, BitArray r)
{
    return BitArray_Or (a, b, r);
}

BitArray BitArray_Intersection (BitArray a, BitArray b, BitArray r)
{
    return BitArray_And (a, b, r);
}

// TODO: implement
BitArray BitArray_SymDifference (BitArray a, BitArray b, BitArray r);

BitArray BitArray_RelDifference (BitArray a, BitArray b, BitArray r)
{
    BitArray not_b = BitArray_Not (b, NULL);
    r = BitArray_And (a, not_b, r);
    BitArray_Delete (not_b);
    return r;
}

/*********************************************************************
 *                             BitMatrix                             *
 *********************************************************************/

BitMatrix BitMatrix_New (int width)
{
    assert (width > 2);

    BitMatrix r = checked_malloc (sizeof (*r));
    r->width = width;
    r->allocated = BitArray_CalculateSize (width) * width;
    r->blob = checked_malloc (r->allocated);
    memset (r->blob, 0, r->allocated);

    // coloring for tests
    /* for (int i = 0; i < r->allocated; ++i) */
    /* { */
    /*     memset (&r->blob[i], i, 1); */
    /* } */

    return r;
}

void BitMatrix_Delete (BitMatrix m)
{
    assert (m);
    free (m->blob);
    free (m);
}

bool BitMatrix_Equal (BitMatrix a, BitMatrix b)
{
    assert (a);
    assert (b);

    int offset = a->allocated;
    while (offset--)
    {
        if (a->blob[offset] != b->blob[offset])
        {
            return FALSE;
        }
    }

    return TRUE;


}

char * BitMatrix_ToString (BitMatrix m)
{
    assert (m);

    /*
     * matrix size plus newline at the end of each row plus null at the end
     * mul by 2 so we make it more "quadratic" in terminal since font usually
     * stretched by y axis
     */
    int ss = m->width * m->width * 2 + m->width + 1;
    char * s = checked_malloc (ss);

    int rp = BitArray_CalculateSize (m->width) * BITS_IN_BYTE;
    int pad = rp - m->width;
    int bits = rp * m->width;
    for (int i = 0; i < bits; ++i)
    {
        int col = i % rp;
        int offset = i / BITS_IN_BYTE;
        int index = i % BITS_IN_BYTE;

        uint8_t byte = m->blob[offset];

        *s++ = ((byte >> index) & 0x01) ? '1' : '0';
        *s++ = ' ';

        if (col == m->width - 1)
        {
            *s++ = '\n';
            i += pad;
        }
    }
    *s++ = '\0';

    return s - ss;
}

void BitMatrix_Print (FILE * out, BitMatrix m)
{
    fprintf (out, "%s\n", BitMatrix_ToString (m));
}

bool BitMatrix_IsSet (BitMatrix m, int row, int collumn)
{
    assert (m);
    assert (row < m->width);
    assert (collumn < m->width);

    int offset = BitArray_CalculateSize (m->width) * row + collumn / BITS_IN_BYTE;
    int shift = collumn % BITS_IN_BYTE;

    return (m->blob[offset] >> shift) & 0x01;
}

void BitMatrix_Set (BitMatrix m, int row, int collumn)
{
    assert (m);
    assert (row < m->width);
    assert (collumn < m->width);

    int offset = BitArray_CalculateSize (m->width) * row + collumn / BITS_IN_BYTE;
    int mask = 0x01 << (collumn % BITS_IN_BYTE);

    m->blob[offset] |= mask;
}

void BitMatrix_SetRow (BitMatrix m, int row)
{
    assert (m);
    assert (row < m->width);

    // FIXME make it right
    BitArray_SetAll (BitMatrix_GetRow (m, row));
}

void BitMatrix_SetColumn (BitMatrix m, int collumn)
{
    assert (m);
    assert (collumn < m->width);

    // row offset in bytes
    int rs = BitArray_CalculateSize (m->width);
    // collumn offset in bytes
    int co = collumn / BITS_IN_BYTE;
    for (int i = 0; i < m->width; ++i)
    {
        int offset = i * rs + co;
        int shift = collumn % BITS_IN_BYTE;
        m->blob[offset] |= (0x01 << shift);
    }
}

void BitMatrix_UnSet (BitMatrix m, int row, int collumn)
{
    assert (m);
    assert (row < m->width);
    assert (collumn < m->width);

    int offset = BitArray_CalculateSize (m->width) * row + collumn / BITS_IN_BYTE;
    int mask = 0x01 << (collumn % BITS_IN_BYTE);

    m->blob[offset] &= ~mask;
}

void BitMatrix_UnSetColumn (BitMatrix m, int collumn)
{
    assert (m);
    assert (collumn < m->width);

    // row offset in bytes
    int rs = BitArray_CalculateSize (m->width);
    // collumn offset in bytes
    int co = collumn / BITS_IN_BYTE;
    for (int i = 0; i < m->width; ++i)
    {
        int offset = i * rs + co;
        int shift = collumn % BITS_IN_BYTE;
        m->blob[offset] &= ~ (0x01 << shift);
    }
}

void BitMatrix_SetColumnFromArray (BitMatrix m, BitArray array, int collumn)
{
    assert (m);
    assert (array);
    assert (m->width == array->length);
    assert (collumn >= 0);

    // row offset in bytes
    int rs = BitArray_CalculateSize (m->width);
    // collumn offset in bytes
    int co = collumn / BITS_IN_BYTE;
    for (int i = 0; i < m->width; ++i)
    {
        int offset = i * rs + co;
        int shift = collumn % BITS_IN_BYTE;
        if (BitArray_IsSet (array, i))
        {
            m->blob[offset] |= (0x01 << shift);
        }
        else
        {
            m->blob[offset] &= ~ (0x01 << shift);
        }
    }
}

BitArray BitMatrix_GetRow (BitMatrix m, int row)
{
    assert (m);
    assert (row < m->width);

    BitArray r = checked_malloc (sizeof (*r));
    r->allocated = BitArray_CalculateSize (m->width);
    r->length = m->width;
    r->blob = &m->blob[BitArray_CalculateSize (m->width) * row];
    r->isView = TRUE;

    return r;
}

BitArray BitMatrix_GetRowCopy (BitMatrix m, int row, BitArray r)
{
    assert (m);
    assert (row < m->width);

    if (r)
    {
        assert (m->width == r->length);
    }
    else
    {
        r = BitArray_New (m->width, FALSE);
    }

    int length = BitArray_CalculateSize (m->width);
    memcpy (r->blob, &m->blob[length * row], length);

    return r;
}

BitArray BitMatrix_GetCollumnCopy (BitMatrix m, int collumn, BitArray r)
{
    assert (m);
    assert (collumn < m->width);

    if (r)
    {
        assert (m->width == r->length);
    }
    else
    {
        r = BitArray_New (m->width, FALSE);
    }

    // row offset in bytes
    int rs = BitArray_CalculateSize (m->width);
    // collumn offset in bytes
    int co = collumn / BITS_IN_BYTE;
    for (int i = 0; i < m->width; ++i)
    {
        int offset = i * rs + co;
        int shift = collumn % BITS_IN_BYTE;
        if ((m->blob[offset] >> shift) & 0x01)
        {
            BitArray_Set (r, i);
        }
        else
        {
            BitArray_UnSet (r, i);
        }
    }

    return r;
}

// TODO: remove unnecessary array creation
int BitMatrix_GetRowDegree (BitMatrix m, int row)
{
    assert (m);
    assert (row >= 0);
    BitArray arow = BitMatrix_GetRow (m, row);
    int r = BitArray_GetDegree (arow);
    BitArray_Delete (arow);
    return r;
}

/*********************************************************************
 *                             Utilities                             *
 *********************************************************************/

static const char * ByteToString (uint8_t x, char * buffer)
{
    int index = BITS_IN_BYTE;
    while (index--)
    {
        buffer[index] = ((x >> index) & 0x01) ? '1' : '0';
    }

    return buffer;
}
