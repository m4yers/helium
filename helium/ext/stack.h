#ifndef STACK_H_JSHQK3ZY
#define STACK_H_JSHQK3ZY

typedef void (*StackItemFree) (void *);

typedef struct
{
    void * items;
    int itemSize;
    StackItemFree itemFree;
    int allocated;
    int length;
} * Stack;

Stack Stack_New (int itemSize, StackItemFree itemFree);
void Stack_Init (Stack s, int itemSize, StackItemFree itemFree);
void Stack_Delete (Stack s);
void Stack_Push (Stack s, const void * item);
void Stack_Pop (Stack s, void * item);

#endif /* end of include guard: STACK_H_JSHQK3ZY */
