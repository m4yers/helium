#include <stdio.h>
#include <string.h>

#include "util/table.h"
#include "util/mem.h"

#include "core/symbol.h"

static S_symbol mksymbol (const char * name, S_symbol next)
{
    S_symbol s = checked_malloc (sizeof (*s));
    s->name = name;
    s->next = next;
    return s;
}

#define SIZE 109  /* should be prime */

static S_symbol hashtable[SIZE];

static unsigned int hash (const char * s0)
{
    unsigned int h = 0;
    const char * s;

    for (s = s0; *s; s++)
    {
        h = h * 65599 + *s;
    }

    return h;
}

static int streq (const char * a, const char * b)
{
    return !strcmp (a, b);
}

S_symbol S_Symbol (const char * name)
{
    int index = hash (name) % SIZE;
    S_symbol syms = hashtable[index], sym;

    for (sym = syms; sym; sym = sym->next)
    {
        if (streq (sym->name, name))
        {
            return sym;
        }
    }

    sym = mksymbol (strdup(name), syms);
    hashtable[index] = sym;
    return sym;
}

const char * S_Name (S_symbol sym)
{
    return sym->name;
}

S_table S_Empty (void)
{
    return TAB_Empty();
}

void S_Enter (S_table t, S_symbol sym, void * value)
{
    TAB_Enter (t, sym, value);
}

const void * S_Look (const S_table t, const S_symbol s)
{
    return TAB_Look (t, s);
}

static struct S_symbol_ marksym = {.name = "<mark>", .next = NULL};

const void * S_LookTop (const S_table t, const S_symbol s)
{
    TAB_FOREACH(k, v, t)
    {
        if (k == &marksym)
        {
            return NULL;
        }
        else if (k == s)
        {
            return v;
        }
    }

    return NULL;
}

void S_BeginScope (S_table t)
{
    S_Enter (t, &marksym, NULL);
}

void S_EndScope (S_table t)
{
    S_symbol s;

    do
    {
        s = TAB_Pop (t);
    }
    while (s != &marksym);
}

void S_Dump (S_table t, void (*show) (const S_symbol sym, const void * binding))
{
    TAB_Dump (t, (void (*) (const void *, const void *)) show);
}
