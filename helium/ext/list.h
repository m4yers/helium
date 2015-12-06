#ifndef LISTS_H_4LNW5ZXX
#define LISTS_H_4LNW5ZXX

#include "mem.h"
#include "bool.h"

#define LIST_LEN(list, result)                                                      \
    result = 0;                                                                     \
    if (list)                                                                       \
    {                                                                               \
        result = 1;                                                                 \
        __typeof__(list) __current = list;                                          \
        while(__current && __current->tail)                                         \
        {                                                                           \
            __current = __current->tail;                                            \
            result++;                                                               \
        }                                                                           \
    }                                                                               \

#define LIST_NEXT(list)                                                             \
    list ? list->tail : NULL                                                        \

#define LIST_BACK(list, var_name)                                                   \
    {                                                                               \
        if (list)                                                                   \
        {                                                                           \
            __typeof__(list) __current = list;                                      \
            while(__current && __current->tail)                                     \
            {                                                                       \
                __current = __current->tail;                                        \
            }                                                                       \
            var_name = __current->head;                                             \
        }                                                                           \
        else                                                                        \
        {                                                                           \
            var_name = NULL;                                                        \
        }                                                                           \
    }                                                                               \

#define LIST_PUSH(list, item)                                                       \
    {                                                                               \
        __typeof__(list) node = checked_malloc(sizeof(*node));                      \
        node->head = item;                                                          \
        node->tail = NULL;                                                          \
        if (list)                                                                   \
        {                                                                           \
            __typeof__(list) __current = list;                                      \
            while(__current && __current->tail)                                     \
            {                                                                       \
                __current = __current->tail;                                        \
            }                                                                       \
            __current->tail = node;                                                 \
        }                                                                           \
        else                                                                        \
        {                                                                           \
            list = node;                                                            \
        }                                                                           \
    }                                                                               \

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

#define LIST_PUSH_UNIQUE(list, item)                                                \
    {                                                                               \
        __typeof__(list) node = checked_malloc(sizeof(*node));                      \
        node->head = item;                                                          \
        node->tail = NULL;                                                          \
        if (list)                                                                   \
        {                                                                           \
            __typeof__(list) __current = list;                                      \
            bool unique = __current->head != item;                                  \
            while(!unique && __current && __current->tail)                          \
            {                                                                       \
                if (__current->head == item)                                        \
                {                                                                   \
                    unique = FALSE;                                                 \
                    break;                                                          \
                }                                                                   \
                __current = __current->tail;                                        \
            }                                                                       \
            if (unique)                                                             \
            {                                                                       \
                __current->tail = node;                                             \
            }                                                                       \
        }                                                                           \
        else                                                                        \
        {                                                                           \
            list = node;                                                            \
        }                                                                           \
    }

#define LIST_INSERT(list, item, index)                                              \
    {                                                                               \
        assert(index >= 0);                                                         \
        __typeof__(list) node = checked_malloc(sizeof(*node));                      \
        node->head = item;                                                          \
        node->tail = NULL;                                                          \
        if (list)                                                                   \
        {                                                                           \
            __typeof__(list) __previous = NULL;                                     \
            __typeof__(list) __current = list;                                      \
            while(index-- != 0 && __current)                                        \
            {                                                                       \
                __previous = __current;                                             \
                __current = __current->tail;                                        \
            }                                                                       \
            node->tail = __current;                                                 \
            if (__previous)                                                         \
            {                                                                       \
                __previous->tail = node;                                            \
            }                                                                       \
        }                                                                           \
        else                                                                        \
        {                                                                           \
            list = node;                                                            \
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

#define LIST_INJECT(list, another, index)                                           \
    {                                                                               \
        if (list)                                                                   \
        {                                                                           \
            int __index = index;                                                    \
            __typeof__(list) __list_inject_previous = NULL;                         \
            __typeof__(list) __list_inject_current = list;                          \
            if (__index < 0)                                                        \
            {                                                                       \
                int len;                                                            \
                LIST_LEN(list, len);                                                \
                __index = MAX(len + __index, 0);                                    \
            }                                                                       \
            while(__index != 0 && __list_inject_current)                            \
            {                                                                       \
                __list_inject_previous = __list_inject_current;                     \
                __list_inject_current = __list_inject_current->tail;                \
                __index--;                                                          \
            }                                                                       \
            if (__list_inject_previous)                                             \
            {                                                                       \
                __list_inject_previous->tail = another;                             \
            }                                                                       \
            else                                                                    \
            {                                                                       \
                list = another;                                                     \
            }                                                                       \
            if (another)                                                            \
            {                                                                       \
                __list_inject_previous = __list_inject_current;                     \
                __list_inject_current = another;                                    \
                while(__list_inject_current && __list_inject_current->tail)         \
                {                                                                   \
                    __list_inject_current = __list_inject_current->tail;            \
                }                                                                   \
                __list_inject_current->tail = __list_inject_previous;               \
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
        __typeof__ (list->head) item = (__##item##__iterator ? __##item##__iterator->head : NULL);\
        __##item##__iterator;                                                       \
        __##item##__iterator = __##item##__iterator->tail,                          \
        item = (__##item##__iterator ? __##item##__iterator->head : NULL))          \

#define LIST_ITEM_AT(list, result, index)                                           \
    {                                                                               \
        int __len = 0;                                                              \
        int __index = index;                                                        \
        LIST_LEN(list, __len);                                                      \
        if (__len <= __index)                                                       \
        {                                                                           \
            result = NULL;                                                          \
        }                                                                           \
        else                                                                        \
        {                                                                           \
            LIST_FOREACH(item, list)                                                \
            {                                                                       \
                if (__index == 0)                                                   \
                {                                                                   \
                    result = item;                                                  \
                    break;                                                          \
                }                                                                   \
                __index--;                                                          \
            }                                                                       \
        }                                                                           \
    }                                                                               \

#define LIST_EVERY(item, list)                                                      \
    LIST_FOREACH(i, list) if (i == item)                                            \

#define LIST_DEFINE(name,type)                                                      \
    typedef struct name##_                                                          \
    {                                                                               \
        type head;                                                                  \
        struct name##_ * tail;                                                      \
    } * name;                                                                       \

LIST_DEFINE (U_boolList, bool)
U_boolList U_BoolList (bool head, U_boolList tail);

LIST_DEFINE (U_stringList, const char *)
U_stringList U_StringList (const char * head, U_stringList tail);

#endif /* end of include guard: LISTS_H_4LNW5ZXX */
