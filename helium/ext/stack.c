#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "stack.h"
#include "mem.h"

#define STACK_FIRST_SIZE 4

Stack Stack_New (int itemSize, StackItemFree itemFree)
{
    assert (itemSize != 0);

    Stack r = checked_malloc (sizeof (*r));
    r->length = 0;
    r->allocated = STACK_FIRST_SIZE * itemSize;
    r->itemSize = itemSize;
    r->itemFree = itemFree;
    r->items = checked_malloc (r->allocated);

    return r;
}

void Stack_Init (Stack s, int itemSize, StackItemFree itemFree)
{
    assert (s);
    assert (s->allocated = 0);
    assert (itemSize != 0);

    s->length = STACK_FIRST_SIZE;
    s->allocated = STACK_FIRST_SIZE * itemSize;
    s->itemSize = itemSize;
    s->itemFree = itemFree;
    s->items = checked_malloc (s->allocated);
}

void Stack_Delete (Stack s)
{
    assert (s);

    if (s->itemFree && s->length)
    {
        while (s->length--)
        {
            s->itemFree ((char *)s->items + s->length * s->itemSize);
        }
    }

    free (s->items);
    free (s);
}

void Stack_Push (Stack s, const void * item)
{
    assert (s);
    assert (item);

    if (s->allocated == s->length * s->itemSize)
    {
        s->allocated += s->itemSize;
        s->items = realloc (s->items, s->allocated);
    }

    memcpy ((char *)s->items + s->length * s->itemSize, item, s->itemSize);

    s->length++;
}

void Stack_Pop (Stack s, void * item)
{
    assert (s);
    assert (s->length != 0);

    s->length--;
    memcpy (item, (char *)s->items + s->length * s->itemSize,  s->itemSize);
}
