#ifndef BITARRAY_H_FZM7HYWQ
#define BITARRAY_H_FZM7HYWQ

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#include "util/bool.h"

/*********************************************************************
 *                             BitArray                              *
 *********************************************************************/

typedef struct
{
    uint8_t * blob;
    int allocated;
    int length;
    bool isView;
} * BitArray;

size_t BitArray_CalculateSize (int bits);

BitArray BitArray_New (int length, bool set);
void BitArray_Delete (BitArray bits);

bool BitArray_Equal (BitArray a, BitArray b);
BitArray BitArray_Clone (BitArray a, BitArray r);
char * BitArray_ToString (BitArray bits);
void BitArray_Print (FILE * out, BitArray bits);

int BitArray_GetDegree (BitArray bits);

bool BitArray_HasSet (BitArray bits);
int BitArray_FirstSet (BitArray bits);
bool BitArray_IsSet (BitArray bits, int index);
void BitArray_Set (BitArray bits, int index);
void BitArray_SetAll (BitArray bits);

void BitArray_UnSet (BitArray bits, int index);
void BitArray_UnSetAll (BitArray bits);

BitArray BitArray_Or (BitArray a, BitArray b, BitArray r);
BitArray BitArray_And (BitArray a, BitArray b, BitArray r);
BitArray BitArray_Xor (BitArray a, BitArray b, BitArray r);
BitArray BitArray_Not (BitArray a, BitArray r);

BitArray BitArray_Union (BitArray a, BitArray b, BitArray r);
BitArray BitArray_Intersection (BitArray a, BitArray b, BitArray r);
BitArray BitArray_SymDifference (BitArray a, BitArray b, BitArray r);
BitArray BitArray_RelDifference (BitArray a, BitArray b, BitArray r);

#define BITARRAY_FOREACH_SET(i, array) \
    for (BitArray __##i##__array = array; __##i##__array; __##i##__array = NULL) \
    for (int i = 0, __##i##__length = __##i##__array->length; i < __##i##__length; i++) \
    if (BitArray_IsSet (__##i##__array, i))

/*********************************************************************
 *                             BitMatrix                             *
 *********************************************************************/

typedef struct
{
    uint8_t * blob;
    int allocated;
    int width;
} * BitMatrix;

/**
 * Creates new bit matrix. The memory matrix occupies is not contiguous and
 * potentially each row can have padding. The reason for this is that you
 * can have live BitArray objects that view a particular row in the matrix.
 */
BitMatrix BitMatrix_New (int width);
void BitMatrix_Delete (BitMatrix m);

bool BitMatrix_Equal (BitMatrix a, BitMatrix b);

char * BitMatrix_ToString (BitMatrix m);
void BitMatrix_Print (FILE * out, BitMatrix m);

bool BitMatrix_IsSet (BitMatrix m, int row, int collumn);
void BitMatrix_Set (BitMatrix m, int row, int collumn);
void BitMatrix_SetRow (BitMatrix m, int row);
void BitMatrix_SetColumn (BitMatrix m, int collumn);
void BitMatrix_UnSet (BitMatrix m, int row, int collumn);
void BitMatrix_UnSetColumn (BitMatrix m, int collumn);
void BitMatrix_SetColumnFromArray (BitMatrix m, BitArray array, int collumn);

BitArray BitMatrix_GetRow (BitMatrix m, int row);
BitArray BitMatrix_GetRowCopy (BitMatrix m, int row, BitArray r);
BitArray BitMatrix_GetCollumnCopy (BitMatrix m, int collumn, BitArray r);

int BitMatrix_GetRowDegree (BitMatrix m, int row);

#endif /* end of include guard: BITARRAY_H_FZM7HYWQ */
