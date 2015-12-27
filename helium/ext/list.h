
/*-----------------------------------------------------------------------------------------,
| LIST:                                                                                    |
|                                                                                          |
| This file contains bunch of macros and inline routines to work with conventional dynamic |
| lists. To make everything work you SHOULD define your list using LIST_DEFINE macro, note |
| that type you provide SHOULD NOT be integer if you intend to look up the elements using  |
| provided macro, like ITEM_AT, because there is no way to return non existing int value.  |
| Normally you SHOULD use these methods to operate on lists of pointer type data.          |
|                                                                                          |
\_________________________________________________________________________________________*/

#ifndef LISTS_H_4LNW5ZXX
#define LISTS_H_4LNW5ZXX

#include <assert.h>

#include "mem.h"
#include "bool.h"
#include "util.h"

#define LIST_DEFINE(name,type)                                                      \
    typedef struct name##_t                                                         \
    {                                                                               \
        /** NOTE: DO NOT FUCKING CHANGE THE ORDER */                                \
        struct name##_t * tail;                                                     \
        type head;                                                                  \
    } * name;                                                                       \

LIST_DEFINE (U_voidList, void *)

static inline size_t List_Size_ (U_voidList list)
{
    size_t r = 0;
    while (list)
    {
        list = list->tail;
        r++;
    }
    return r;
}

#define LIST_SIZE(list) List_Size_((U_voidList) list)

#define LIST_NEXT(list) list ? list->tail : NULL                                    \

static inline U_voidList List_Back_ (U_voidList list)
{
    while (list && list->tail)
    {
        list = list->tail;
    }
    return list;
}

#define LIST_BACK(list) (((__typeof__(list))List_Back_((U_voidList) list))->head)

static inline U_voidList List_At_ (U_voidList list, size_t pos)
{
    while (pos--)
    {
        list = list->tail;
    }
    return list;
}

#define LIST_AT(list, pos) (((__typeof__(list))List_At_((U_voidList)list,pos))->head)

#define LIST_PUSH(list, item)                                                       \
    {                                                                               \
        __typeof__(list) __node = checked_malloc(sizeof(*__node));                  \
        __node->head = item;                                                        \
        __node->tail = NULL;                                                        \
        if (list)                                                                   \
        {                                                                           \
            __typeof__(list) __current = list;                                      \
            while(__current && __current->tail)                                     \
            {                                                                       \
                __current = __current->tail;                                        \
            }                                                                       \
            __current->tail = __node;                                               \
        }                                                                           \
        else                                                                        \
        {                                                                           \
            list = __node;                                                          \
        }                                                                           \
    }                                                                               \

#define LIST_PUSH_UNIQUE(list, item)                                                \
    {                                                                               \
        __typeof__(list) __node = checked_malloc(sizeof(*__node));                  \
        __node->head = item;                                                        \
        __node->tail = NULL;                                                        \
        if (list)                                                                   \
        {                                                                           \
            __typeof__(list) __current = list;                                      \
            bool unique = __current->head != item;                                  \
            while(__current && __current->tail)                                     \
            {                                                                       \
                if (__current->head == item)                                        \
                {                                                                   \
                    unique = FALSE;                                                 \
                }                                                                   \
                __current = __current->tail;                                        \
            }                                                                       \
            if (unique && __current->head != item)                                  \
            {                                                                       \
                __current->tail = __node;                                           \
            }                                                                       \
        }                                                                           \
        else                                                                        \
        {                                                                           \
            list = __node;                                                          \
        }                                                                           \
    }

#define LIST_REMOVE(list, item)                                                     \
    {                                                                               \
        if (list)                                                                   \
        {                                                                           \
            __typeof__(list) __current = list;                                      \
            __typeof__(list) __previous = NULL;                                     \
            while(__current)                                                        \
            {                                                                       \
                if (__current->head == item)                                        \
                {                                                                   \
                    if (__previous)                                                 \
                    {                                                               \
                        __previous->tail = __current->tail;                         \
                    }                                                               \
                    else                                                            \
                    {                                                               \
                        list = __current->tail;                                     \
                    }                                                               \
                    break;                                                          \
                }                                                                   \
                __previous = __current;                                             \
                __current = __current->tail;                                        \
            }                                                                       \
        }                                                                           \
    }

#define LIST_JOIN(list, another)                                                    \
    {                                                                               \
        if (list)                                                                   \
        {                                                                           \
            __typeof__(list) __list_join_current = list;                            \
            while(__list_join_current && __list_join_current->tail)                 \
            {                                                                       \
                __list_join_current = __list_join_current->tail;                    \
            }                                                                       \
            __list_join_current->tail = another;                                    \
        }                                                                           \
        else                                                                        \
        {                                                                           \
            list = another;                                                         \
        }                                                                           \
    }

#define LIST_INSERT(list, item, index)                                              \
    {                                                                               \
        assert(index >= 0);                                                         \
        __typeof__(index) __index = index;                                          \
        __typeof__(list) __node = checked_malloc(sizeof(*__node));                  \
        __node->head = item;                                                        \
        __node->tail = NULL;                                                        \
        if (list)                                                                   \
        {                                                                           \
            __typeof__(list) __previous = NULL;                                     \
            __typeof__(list) __current = list;                                      \
            while(__index != 0 && __current)                                        \
            {                                                                       \
                __previous = __current;                                             \
                __current = __current->tail;                                        \
                __index--;                                                          \
            }                                                                       \
            __node->tail = __current;                                               \
            if (__previous)                                                         \
            {                                                                       \
                __previous->tail = __node;                                          \
            }                                                                       \
            else                                                                    \
            {                                                                       \
                list = __node;                                                      \
            }                                                                       \
        }                                                                           \
        else                                                                        \
        {                                                                           \
            list = __node;                                                          \
        }                                                                           \
    }

#define LIST_INJECT(list, another, index)                                           \
    {                                                                               \
        if (list)                                                                   \
        {                                                                           \
            __typeof__(index) __index = index;                                      \
            __typeof__(list) __previous = NULL;                                     \
            __typeof__(list) __current = list;                                      \
            /** index counting from the end */                                      \
            if (__index < 0)                                                        \
            {                                                                       \
                size_t len = LIST_SIZE(list);                                       \
                __index = MAX(len + __index, 0);                                    \
            }                                                                       \
            while(__index != 0 && __current)                                        \
            {                                                                       \
                __previous = __current;                                             \
                __current = __current->tail;                                        \
                __index--;                                                          \
            }                                                                       \
            if (__previous)                                                         \
            {                                                                       \
                __previous->tail = another;                                         \
            }                                                                       \
            else                                                                    \
            {                                                                       \
                list = another;                                                     \
            }                                                                       \
            if (another)                                                            \
            {                                                                       \
                __previous = __current;                                             \
                __current = another;                                                \
                while(__current && __current->tail)                                 \
                {                                                                   \
                    __current = __current->tail;                                    \
                }                                                                   \
                __current->tail = __previous;                                       \
            }                                                                       \
        }                                                                           \
        else                                                                        \
        {                                                                           \
            list = another;                                                         \
        }                                                                           \
    }

#define LIST_FOREACH(item, list)                                                    \
    for (                                                                           \
            __typeof__ (list) __##item##__iterator = list;                          \
            __##item##__iterator;                                                   \
            __##item##__iterator = NULL)                                            \
    for (                                                                           \
        __typeof__ (list->head) item = (__##item##__iterator ? __##item##__iterator->head : 0);\
        __##item##__iterator;                                                       \
        __##item##__iterator = __##item##__iterator->tail,                          \
        item = (__##item##__iterator ? __##item##__iterator->head : 0))             \

LIST_DEFINE (U_boolList, bool)
U_boolList U_BoolList (bool head, U_boolList tail);

LIST_DEFINE (U_stringList, const char *)
U_stringList U_StringList (const char * head, U_stringList tail);

#endif /* end of include guard: LISTS_H_4LNW5ZXX */
