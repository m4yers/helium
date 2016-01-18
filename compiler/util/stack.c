#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "util/stack.h"
#include "util/mem.h"

void Stack_Init (Stack s, size_t itemSize, StackItemDest itemFree)
{
    assert (s);

    s->size = 0;
    s->allocated = STACK_INIT_ALLOC * itemSize;
    s->itemSize = itemSize;
    s->itemFree = itemFree;
    s->items = checked_malloc (s->allocated);
}

void Stack_Fini (Stack s)
{
    assert (s);

    if (s->itemFree && s->size)
    {
        while (s->size--)
        {
            s->itemFree ((char *)s->items + s->size * s->itemSize);
        }
    }

    free (s->items);
}

size_t Stack_Size (Stack s)
{
    assert (s);
    return s->size;
}

bool Stack_Empty (Stack s)
{
    assert (s);
    return s->size == 0;
}

void * Stack_Top (Stack s)
{
    assert (s);
    assert (s->size != 0);

    return s->items + (s->size - 1) * s->itemSize;
}

void Stack_Pop (Stack s)
{
    assert (s);
    assert (s->size != 0);

    s->size--;
}
