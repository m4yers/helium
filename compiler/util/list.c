#include "util/list.h"
#include "util/mem.h"

U_boolList U_BoolList (bool head, U_boolList tail)
{
    U_boolList list = checked_malloc (sizeof (*list));
    list->head = head;
    list->tail = tail;
    return list;
}

U_stringList U_StringList (const char * head, U_stringList tail)
{
    U_stringList list = checked_malloc (sizeof (*list));
    list->head = head;
    list->tail = tail;
    return list;
}
