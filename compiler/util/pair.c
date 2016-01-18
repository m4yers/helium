#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "util/mem.h"
#include "util/pair.h"

void Pair_Init_ (Pair p, size_t fs, const void * fd, size_t ss, const void * sd)
{
    assert (p);
    assert (fs > 0);
    assert (fd);
    assert (ss > 0);
    assert (sd);

    p->first = checked_malloc (fs + ss);
    p->second = (char *)p->first + fs;

    memcpy ((void *)p->first, fd, fs);
    memcpy ((void *)p->second, sd, ss);
}

void Pair_Fini (Pair p)
{
    assert (p);

    free ((void *)p->first);

    p->first = NULL;
    p->second = NULL;
}

Pair Pair_New_ (size_t fs, const void * fd, size_t ss, const void * sd)
{
    Pair p = checked_malloc (sizeof (*p));
    Pair_Init_ (p, fs, fd, ss, sd);
    return p;
}

const void * Pair_First (Pair p)
{
    return p->first;
}

const void * Pair_Second (Pair p)
{
    return p->second;
}
