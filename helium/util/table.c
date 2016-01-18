#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "util/mem.h"
#include "util/table.h"

static record Record (const void * key, const void * value, record next, record prev_added)
{
    record b = checked_malloc (sizeof (*b));
    b->key = key;
    b->value = value;
    b->next = next;
    b->prev_added = prev_added;
    return b;
}

TAB_table TAB_Empty (void)
{
    TAB_table t = checked_malloc (sizeof (*t));
    int i;
    t->top = NULL;

    for (i = 0; i < TABSIZE; i++)
    {
        t->table[i] = NULL;
    }

    return t;
}

/* The cast from pointer to integer in the expression
 *   ((unsigned)key) % TABSIZE
 * may lead to a warning message.  However, the code is safe,
 * and will still operate correctly.  This line is just hashing
 * a pointer value into an integer value, and no matter how the
 * conversion is done, as long as it is done consistently, a
 * reasonable and repeatable index into the table will result.
 */

void TAB_Enter (TAB_table t, const void * key, const void * value)
{
    assert (t);
    assert (key);

    int index = ((unsigned)key) % TABSIZE;
    record prev_added = t->top ? t->table[ ((unsigned)t->top) % TABSIZE] : NULL;
    t->table[index] = Record (key, value, t->table[index], prev_added);
    t->top = key;
}

const void * TAB_Look (TAB_table t, const void * key)
{
    int index;
    record b;
    assert (t);
    assert (key);
    index = ((unsigned)key) % TABSIZE;

    for (b = t->table[index]; b; b = b->next)
        if (b->key == key)
        {
            return b->value;
        }

    return NULL;
}

void * TAB_Pop (TAB_table t)
{
    const void * k;
    record b;
    int index;
    assert (t);
    k = t->top;
    assert (k);
    index = ((unsigned)k) % TABSIZE;
    b = t->table[index];
    assert (b);
    t->table[index] = b->next;
    t->top = b->prev_added ? b->prev_added->key : NULL;
    return (void *)b->key;
}

void ** TAB_Keys (TAB_table t)
{
    int size = 10;
    void ** r = checked_malloc (sizeof (void *) * size);
    int i = 0;
    const void * hash = t->top;
    while (hash)
    {
        int index = ((unsigned)hash) % TABSIZE;
        record b = t->table[index];
        if (i == size)
        {
            size *= 2;
            r = realloc (r, sizeof (void *) * size);
        }
        r[i++] = (void *) b->key;
    }

    return r;
}

void TAB_Dump (TAB_table t, void (*show) (const void * key, const void * value))
{
    const void * k = t->top;
    int index = ((unsigned)k) % TABSIZE;
    record b = t->table[index];

    if (b == NULL)
    {
        return;
    }

    t->table[index] = b->next;
    t->top = b->prev_added ? b->prev_added->key : NULL;
    show (b->key, b->value);
    TAB_Dump (t, show);
    assert (t->top == b->prev_added->key && t->table[index] == b->next);
    t->top = k;
    t->table[index] = b;
}
