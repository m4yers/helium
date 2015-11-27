#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "vector.h"
#include "mem.h"
#include "bool.h"
#include "log.h"

Vector _Vector_Init (Vector v, size_t type_size, size_t n)
{
    assert (v);
    assert (type_size != 0);

    v->allocated = type_size * n;
    // HMM... should it be aligned by type_size
    v->data = checked_malloc (v->allocated);
    v->type_size = type_size;
    v->capacity = n;
    v->size = 0;
    v->dest = NULL;

    return v;
}

void Vector_Fini (Vector v)
{
    assert (v);

    Vector_Clear (v);

    free (v->data);
}

Vector Vector_New (size_t type_size, size_t n)
{
    Vector v = checked_malloc (sizeof (*v));
    return _Vector_Init (v, type_size, n);
}

void Vector_Delete (Vector v)
{
    Vector_Fini (v);
    free (v);
}

void Vector_SetDestructor (Vector v, VectorElementDestructor f)
{
    assert (v);
    v->dest = f;
}

bool Vector_Empty (Vector v)
{
    assert (v);
    return v->size == 0;
}

void Vector_Clear (Vector v)
{
    assert (v);

    if (v->dest)
    {
        for (size_t i = 0; i < v->size; ++i)
        {
            v->dest (v->data + v->type_size * i);
        }
    }

    v->size = 0;
}

void Vector_Reserve (Vector v, size_t n)
{
    assert (v);
    assert (n > 0);

    if (v->capacity < n)
    {
        v->capacity = n;
        // FIXME what if there is no memory? use temp
        v->data = realloc (v->data, v->capacity * v->type_size);
    }
}

void Vector_Resize (Vector v, size_t capacity, const void * value)
{
    // TODO make it happen
    assert (0);
    (void) v;
    (void) capacity;
    (void) value;
}

void Vector_Push (Vector v, const void * value)
{
    assert (v);

    if (v->size == v->capacity)
    {
        Vector_Reserve (v, v->capacity * 2);
    }

    memcpy (v->data + v->type_size * v->size, value, v->type_size);
    v->size++;
}

void * Vector_Front (const Vector v)
{
    assert (v);
    assert (v->size != 0);
    return v->data;
}

void * Vector_Back (const Vector v)
{
    assert (v);
    assert (v->size != 0);
    return v->data + (v->size - 1) * v->type_size;
}

void Vector_PopBack (Vector v)
{
    assert (v);
    assert (v->size != 0);
    v->size--;
    if (v->dest)
    {
        v->dest (v->data + v->size * v->type_size);
    }
}

size_t Vector_Size (Vector v)
{
    assert (v);
    return v->size;
}

size_t Vector_Capacity (Vector v)
{
    assert (v);
    return v->capacity;
}

void * Vector_At (Vector v, size_t pos)
{
    assert (v);
    assert (pos < v->size);

    return v->data + v->type_size * pos;
}

void * Vector_Data (Vector v)
{
    assert (v);
    return v->data;
}
