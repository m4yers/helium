#include <assert.h>

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

Temp_temp F_RegistersGet(F_registers regs, int index)
{
    assert (index < regs->number);

    for (Temp_tempList l = regs->temps; l; l = l->tail)
    {
        if (index-- == 0)
        {
            return l->head;
        }
    }

    assert(0);
    return NULL;
}

bool F_RegistersContains(F_registers regs, Temp_temp reg)
{
    assert (reg);

    LIST_FOREACH(item, regs->temps)
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
            // TODO: Artyom Goncharov remove cast and
            Temp_Enter (map, l->head, (char *) s->head);
        }
    }

    return map;
}
