#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "str.h"

#include "ext/mem.h"
#include "ext/bool.h"

String String_Init (String s, const char * data)
{
    assert (s);
    assert (data);

    s->data = (char *) data;
    s->is_static = TRUE;
    s->size = strlen (data);
    s->capacity = s->size + 1;

    return s;
}

void String_Fini (String s)
{
    assert (s);
    assert (s->data);

    if (!s->is_static)
    {
        free (s->data);
    }

    s->data = NULL;
    s->capacity = 0;
    s->size = 0;
    s->is_static = FALSE;
}

String String_New (const char * data)
{
    String r = checked_malloc (sizeof (*r));
    return String_Init (r, data);
}

void String_NoStatic (String s)
{
    assert (s);
    if (s->is_static)
    {
        const char * old = s->data;
        s->data = checked_malloc (s->size);
        memcpy (s->data, old, s->size + 1);
        s->is_static = FALSE;
    }
}

bool String_IsStatic (String s)
{
    assert (s);
    return s->is_static;
}

bool String_Empty (const String s)
{
    assert (s);
    return s->size == 0;
}

void String_Clear (String s)
{
    assert (s);

    if (s->is_static)
    {
        s->data = NULL;
        s->size = 0;
        s->capacity = 0;
        s->is_static = FALSE;
    }
    else
    {
        s->size = 0;
    }
}

void String_Reserve (String s, size_t n)
{
    assert (s);
    assert (n > 0);

    if (s->capacity < n)
    {
        s->capacity = n;
        if (s->is_static)
        {
            const char * old = s->data;
            s->data = checked_malloc (n);
            memcpy (s->data, old, s->size + 1);
            s->is_static = FALSE;
        }
        else
        {
            // FIXME what if there is no memory? use temp
            s->data = realloc (s->data, n);
        }
    }
}

void String_Resize (String s, size_t n, char c)
{
    // TODO make it happen
    assert (0);
    (void) s;
    (void) n;
    (void) c;
}

void String_PushBack (String s, char c)
{
    assert (s);

    String_NoStatic (s);

    if (s->size + 1 == s->capacity)
    {
        // TODO we need a better grow strategy here
        String_Reserve (s, s->capacity * 2);
    }

    s->data[s->size++] = c;
    s->data[s->size] = '\0';
}

void String_PopBack (String s)
{
    assert (s);
    assert (s->size != 0);
    String_NoStatic (s);
    s->data[--s->size] = '\0';
}

const char * String_Front (const String s)
{
    assert (s);
    assert (s->size != 0);
    return s->data;
}

const char * String_Back (const String s)
{
    assert (s);
    assert (s->size != 0);
    return s->data + (s->size - 1);
}

size_t String_Size (const String s)
{
    assert (s);
    return s->size;
}

size_t String_Capacity (const String s)
{
    assert (s);
    return (s->capacity);
}

const void * String_At (const String s, size_t pos)
{
    assert (s);
    assert (pos < s->size);
    return s->data + pos;
}

void * String_Data (const String s)
{
    assert (s);
    return s->data;
}
