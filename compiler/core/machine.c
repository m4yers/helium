#include <assert.h>
#include <string.h>

#include "util/list.h"
#include "util/mem.h"

#include "core/machine.h"

M_regs M_Regs (Temp_tempList temps, U_stringList names)
{
    M_regs r = checked_malloc (sizeof (*r));
    r->number = 0;
    r->temps = r->last_temp = temps;
    r->names = r->last_name = names;
    return r;
}

Temp_temp M_RegGet_u (M_regs regs, int index)
{
    assert (index < regs->number);

    LIST_FOREACH(l, regs->temps)
    {
        if (index-- == 0)
        {
            return l;
        }
    }

    assert (0);
    return NULL;
}

Temp_temp M_RegGet_s (M_regs regs, const char * name)
{
    assert (name);

    U_stringList s = regs->names;
    LIST_FOREACH(t, regs->temps)
    {
        const char * n = s->head;
        if (strcmp (n, name) == 0)
        {
            return t;
        }

        s = LIST_NEXT(s);
    }

    return NULL;
}

const char * M_RegGetName (M_regs regs, int index)
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

bool M_RegsHas (M_regs regs, Temp_temp reg)
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

void M_RegsAdd (M_regs regs, Temp_temp temp, const char * name)
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

Temp_map M_RegsToTempMap (Temp_map map, M_regs regs)
{
    if (regs)
    {
        U_stringList s = regs->names;
        LIST_FOREACH(t, regs->temps)
        {
            Temp_Enter (map, t, s->head);
            s = LIST_NEXT(s);
        }
    }

    return map;
}
