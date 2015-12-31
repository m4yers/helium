#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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
        s->data = strdup (s->data);
        s->is_static = FALSE;
    }
}

bool String_IsStatic (const struct String_t * s)
{
    assert (s);
    return s->is_static;
}

bool String_Empty (const struct String_t * s)
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

int String_Diff_s (const struct String_t * a, const struct String_t * b)
{
    assert (a);
    assert (b);

    char * ad = a->data;
    char * bd = b->data;
    for (int c = 0;; c++, ad++, bd++)
    {
        if (*ad != *bd)
        {
            return c;
        }
        else if (*ad == '\0')
        {
            return -1;
        }
    }

    return -1;
}

int String_Diff_cp (const struct String_t * a, const char * b)
{
    assert (a);
    assert (b);

    char * ad = a->data;
    char * bd = (char *)b;
    for (int c = 0;; c++, ad++, bd++)
    {
        if (*ad != *bd)
        {
            return c;
        }
        else if (*ad == '\0')
        {
            return -1;
        }
    }

    return -1;
}

bool String_Equal_s (const struct String_t * a, const struct String_t * b)
{
    return String_Diff (a, b) == -1;
}

bool String_Equal_c (const struct String_t * a, const char * b)
{
    return String_Diff_cp (a, b) == -1;
}

int String_Cmp (const struct String_t * a, const struct String_t * b)
{
    assert (a);
    assert (b);
    return strcmp (a->data, b->data);
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

const char * String_Front (const struct String_t * s)
{
    assert (s);
    assert (s->size != 0);
    return s->data;
}

const char * String_Back (const struct String_t * s)
{
    assert (s);
    assert (s->size != 0);
    return s->data + (s->size - 1);
}

void String_Append_cp (String s, const char * c)
{
    assert (s);
    assert (c);

    size_t size = strlen (c);
    if (size == 0)
    {
        return;
    }

    String_Reserve (s, s->size + size + 1);

    memcpy (s->data + s->size, c, size);

    s->size += size;
    s->data[s->size] = '\0';
}

void String_Append_s (String s, const struct String_t * o)
{
    assert (s);
    assert (o);

    String_Append_cp (s, o->data);
}

void String_Append_l (String s, long n)
{
    assert (s);
    char str[256];
    sprintf (str, "%lu", n);
    String_Append_cp (s, str);
}

Vector String_Split_s (const struct String_t * s, const struct String_t * d)
{
    assert (s);
    assert (d);
    assert (0);
}

Vector String_Split_c (const struct String_t * s, char d)
{
    assert (s);

    Vector vec = Vector_New (struct String_t);

    if (String_Size (s) == 0)
    {
        return vec;
    }

    struct String_t str = String ("");

    STRING_FOREACH (c, s)
    {
        if (c == d)
        {
            Vector_Push (vec, &str);
            String_Init (&str, "");
        }
        else
        {
            String_PushBack (&str, c);
        }
    }

    Vector_Push (vec, &str);

    return vec;
}

Vector String_Split_cp (const struct String_t * s, const char * d)
{
    assert (s);
    assert (d);
    assert (0);
}

void String_Assign_s (String s, const struct String_t * o)
{
    assert (s);
    assert (o);

    s->data = strdup (o->data);
    s->size = o->size;
    s->capacity = o->capacity;
    s->is_static = FALSE;
}

void String_Assign_cp (String s, const char * c)
{
    assert (s);
    assert (c);

    s->data = strdup (c);
    s->size = strlen (c);
    s->capacity = s->size + 1;
    s->is_static = FALSE;
}

void String_Insert_cp (String s, size_t pos, const char * c)
{
    assert (s);
    assert (c);
    assert (pos <= s->size);

    size_t size = strlen (c);

    if (size == 0)
    {
        return;
    }

    String_Reserve (s, s->size + size + 1);

    memmove (s->data + pos + size, s->data + pos, s->size - pos + 1);
    memcpy (s->data + pos, c, size);
    s->size += size;
}

size_t String_Size (const struct String_t * s)
{
    assert (s);
    return s->size;
}

size_t String_Capacity (const struct String_t * s)
{
    assert (s);
    return (s->capacity);
}

const char * String_At (const struct String_t * s, size_t pos)
{
    assert (s);
    assert (pos < s->size);
    return s->data + pos;
}

void * String_Data (const struct String_t * s)
{
    assert (s);
    return s->data;
}
