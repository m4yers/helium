#ifndef STACK_H_JSHQK3ZY
#define STACK_H_JSHQK3ZY

#define STACK_INIT_ALLOC 4

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ext/mem.h"
#include "ext/bool.h"

typedef void (*StackItemDest) (void *);

typedef struct Stack_t
{
    char * items;
    size_t itemSize;
    StackItemDest itemFree;
    size_t allocated;
    size_t size;
} * Stack;

void Stack_Init (Stack s, size_t itemSize, StackItemDest itemFree);
void Stack_Fini (Stack s);
size_t Stack_Size (Stack s);
bool Stack_Empty (Stack s);
void * Stack_Top (Stack s);
void Stack_Pop (Stack s);

#define Stack(type,dest)                                                                  \
{                                                                                         \
    .items = checked_malloc(STACK_INIT_ALLOC * sizeof(type)),                             \
    .itemSize = sizeof(type),                                                             \
    .itemFree = dest,                                                                     \
    .allocated = STACK_INIT_ALLOC * sizeof(type),                                         \
    .size = 0                                                                             \
}

static inline Stack Stack_New_ (int itemSize, StackItemDest itemFree)
{
    assert (itemSize != 0);

    Stack r = checked_malloc (sizeof (*r));
    Stack_Init (r, itemSize, itemFree);
    return r;
}

#define Stack_New(type,dest) Stack_New_(sizeof(type),dest)
#define Stack_Delete(s) Stack_Fini(s); free(s); s = NULL;

static inline void Stack_Push_ (Stack s, const void * item)
{
    assert (s);
    assert (item);

    if (s->allocated == s->size * s->itemSize)
    {
        s->allocated += s->itemSize;
        s->items = realloc (s->items, s->allocated);
    }

    memcpy ((char *)s->items + s->size * s->itemSize, item, s->itemSize);

    s->size++;
}

#define Stack_Push(stack, value)                                                          \
{                                                                                         \
    __typeof__(TRUE ? value : value) __value = value;                                     \
    Stack_Push_ (stack, &__value);                                                        \
}

#define STACK_FOREACH(type, item, stack)                                                  \
    for (Stack __s = stack; __s; __s = NULL)                                              \
    for (size_t __i = 0, __size = __s->size; __i < __size; ++__i)                         \
    for (type * item = (type *)(__s->items + __i * __s->itemSize);item;item = NULL)

#define STACK_FOREACH_BACKWARDS(type, item, stack)                                        \
    for (Stack __s = stack; __s; __s = NULL)                                              \
    for (size_t __i = 0, __size = __s->size; __i < __size; ++__i)                         \
    for (type * item = (type *)(__s->items + (__s->size -__i - 1) * __s->itemSize);item;item = NULL)

#endif /* end of include guard: STACK_H_JSHQK3ZY */
