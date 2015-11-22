#include <stdlib.h>
#include <stdio.h>

#include "mem.h"

void * aligned_malloc (size_t required_bytes, size_t alignment)
{
    void * p1;
    void ** p2;
    int offset = alignment - 1 + sizeof (void *);
    p1 = malloc (required_bytes + offset);
    if ((p1 = (void *)checked_malloc (required_bytes + offset)) == NULL)
    {
        return NULL;
    }
    p2 = (void **) (((size_t) (p1) + offset) & ~ (alignment - 1));
    p2[-1] = p1;
    return p2;
}

void aligned_free (void * p)
{
    void * p1 = ((void **)p)[-1];
    free (p1);
}

void * checked_malloc (int len)
{
    void * p = malloc (len);

    if (!p)
    {
        fprintf (stderr, "\nRan out of memory!\n");
        exit (1);
    }

    return p;
}
