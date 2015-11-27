#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "mem.h"
#include "table.h"

#define TABSIZE 127

typedef struct binder_ * binder;
struct binder_
{
    const void * key;
    const void * value;
    binder next;
    const void * prevtop;
};

struct TAB_table_
{
    binder table[TABSIZE];
    const void * top;
};

static binder Binder (const void * key, const void * value, binder next, const void * prevtop)
{
    binder b = checked_malloc (sizeof (*b));
    b->key = key;
    b->value = value;
    b->next = next;
    b->prevtop = prevtop;
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
    int index;
    assert (t);
    assert (key);
    index = ((unsigned)key) % TABSIZE;
    t->table[index] = Binder (key, value, t->table[index], t->top);
    t->top = key;
}

const void * TAB_Look (TAB_table t, const void * key)
{
    int index;
    binder b;
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
    binder b;
    int index;
    assert (t);
    k = t->top;
    assert (k);
    index = ((unsigned)k) % TABSIZE;
    b = t->table[index];
    assert (b);
    t->table[index] = b->next;
    t->top = b->prevtop;
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
        binder b = t->table[index];
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
    binder b = t->table[index];

    if (b == NULL)
    {
        return;
    }

    t->table[index] = b->next;
    t->top = b->prevtop;
    show (b->key, b->value);
    TAB_Dump (t, show);
    assert (t->top == b->prevtop && t->table[index] == b->next);
    t->top = k;
    t->table[index] = b;
}
