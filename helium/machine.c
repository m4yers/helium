#include <assert.h>
#include <string.h>

#include "ext/list.h"
#include "ext/mem.h"

#include "machine.h"

F_registers F_Registers (Temp_tempList temps, U_stringList names)
{
    F_registers r = checked_malloc (sizeof (*r));
    r->number = 0;
    r->temps = r->last_temp = temps;
    r->names = r->last_name = names;
    return r;
}

Temp_temp F_RegistersGet (F_registers regs, int index)
{
    assert (index < regs->number);

    for (Temp_tempList l = regs->temps; l; l = l->tail)
    {
        if (index-- == 0)
        {
            return l->head;
        }
    }

    assert (0);
    return NULL;
}

const char * F_RegistersGetName (F_registers regs, int index)
{
    assert (index >= 0);
    assert (index < regs->number);

    LIST_FOREACH (s, regs->names)
    {
        if (index-- == 0)
        {
            return s;
        }
    }

    return NULL;
}

Temp_temp F_RegistersGet_s (F_registers regs, const char * name)
{
    assert (name);

    Temp_tempList t = regs->temps;
    U_stringList l = regs->names;
    for (; l; t = t->tail, l = l->tail)
    {
        const char * n = l->head;
        if (strcmp (n, name) == 0)
        {
            return t->head;
        }
    }

    return NULL;
}

bool F_RegistersContains (F_registers regs, Temp_temp reg)
{
    assert (reg);

    LIST_FOREACH (item, regs->temps)
    {
        if (item == reg)
        {
            return TRUE;
        }
    }

    return FALSE;
}

void F_RegistersAdd (F_registers regs, Temp_temp temp, const char * name)
{
    if (!regs->temps)
    {
        regs->temps = regs->last_temp = Temp_TempList (temp, NULL);
        regs->names = regs->last_name = U_StringList (name, NULL);
        regs->number++;
        return;
    }

    regs->last_temp = regs->last_temp->tail = Temp_TempList (temp, NULL);
    regs->last_name = regs->last_name->tail = U_StringList (name, NULL);
    regs->number++;
}

Temp_map F_RegistersToMap (Temp_map map, F_registers regs)
{
    if (regs)
    {
        Temp_tempList l = regs->temps;
        U_stringList s = regs->names;
        for (; l; l = l->tail, s = s->tail)
        {
            Temp_Enter (map, l->head, s->head);
        }
    }

    return map;
}
