#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ext/bitarray.h"
#include "ext/table.h"
#include "ext/util.h"
#include "ext/mem.h"

#include "temp.h"

struct Temp_temp_
{
    int num;
};

const char * Temp_LabelString (Temp_label s)
{
    return S_Name (s);
}

static int labels = 0;

Temp_label Temp_NewLabel (void)
{
    char buf[100];
    sprintf (buf, "L%d", labels++);
    return Temp_NamedLabel (buf);
}

/* The label will be created only if it is not found. */
Temp_label Temp_NamedLabel (const char * s)
{
    return S_Symbol (s);
}

static int temps = 100;

Temp_temp Temp_NewTemp (void)
{
    Temp_temp p = (Temp_temp) checked_malloc (sizeof (*p));
    p->num = temps++;
    {
        char r[16];
        sprintf (r, "t%d", p->num);
        Temp_Enter (Temp_Name(), p, r);
    }
    return p;
}



struct Temp_map_
{
    TAB_table tab;
    Temp_map under;
};


Temp_map Temp_Name (void)
{
    static Temp_map m = NULL;
    if (!m)
    {
        m = Temp_Empty();
    }
    return m;
}

static Temp_map newMap (TAB_table tab, Temp_map under)
{
    Temp_map m = checked_malloc (sizeof (*m));
    m->tab = tab;
    m->under = under;
    return m;
}

Temp_map Temp_Empty (void)
{
    return newMap (TAB_Empty(), NULL);
}

Temp_map Temp_LayerMap (Temp_map over, Temp_map under)
{
    if (over == NULL)
    {
        return under;
    }
    else
    {
        return newMap (over->tab, Temp_LayerMap (over->under, under));
    }
}

void Temp_Enter (Temp_map m, Temp_temp t, char * s)
{
    assert (m && m->tab);
    TAB_Enter (m->tab, t, s);
}

const char * Temp_Look (Temp_map m, Temp_temp t)
{
    char * s;
    assert (m && m->tab);
    s = TAB_Look (m->tab, t);
    if (s)
    {
        return s;
    }
    else if (m->under)
    {
        return Temp_Look (m->under, t);
    }
    else
    {
        return NULL;
    }
}

Temp_tempList Temp_TempList (Temp_temp h, Temp_tempList t)
{
    Temp_tempList p = (Temp_tempList) checked_malloc (sizeof (*p));
    p->head = h;
    p->tail = t;
    return p;
}

int Temp_GetTempIndex(Temp_temp temp)
{
    return temp->num;
}

Temp_tempList Temp_SortTempList(Temp_tempList list)
{
    if (list == NULL || list->tail == NULL)
        return list;

    Temp_tempList head = NULL;
    while (list != NULL)
    {
        Temp_tempList current = list;
        list = list->tail;
        if (head == NULL || current->head->num < head->head->num)
        {
            current->tail = head;
            head = current;
        }
        else
        {
            Temp_tempList p = head;
            while (p != NULL)
            {
                if (p->tail == NULL || current->head->num < p->tail->head->num)
                {
                    current->tail = p->tail;
                    p->tail = current;
                    break;
                }
                p = p->tail;
            }
        }
    }

    return head;
}

bool Temp_IsTempInList (Temp_tempList list, Temp_temp temp)
{
    assert (list);
    assert (temp);

    while (list)
    {
        if (list->head == temp)
        {
            return TRUE;
        }
        list = list->tail;
    }

    return FALSE;
}

Temp_labelList Temp_LabelList (Temp_label h, Temp_labelList t)
{
    Temp_labelList p = (Temp_labelList) checked_malloc (sizeof (*p));
    p->head = h;
    p->tail = t;
    return p;
}

static FILE * outfile;
static void showit (Temp_temp t, char * r)
{
    fprintf (outfile, "t%d -> %s\n", t->num, r);
}

void Temp_DumpMap (FILE * out, Temp_map m)
{
    outfile = out;
    TAB_Dump (m->tab, (void (*) (void *, void *))showit);
    if (m->under)
    {
        fprintf (out, "---------\n");
        Temp_DumpMap (out, m->under);
    }
}

Temp_temp GetTempFromList (Temp_tempList list, int index)
{
    assert (list);
    assert (index >= 0);

    while (list)
    {
        if (index == 0)
        {
            return list->head;
        }
        list = list->tail;
        index--;
    }

    return NULL;
}

BitArray MapTempList (int length, Temp_tempList subset, Temp_tempList set)
{
    BitArray r = BitArray_New (length, FALSE);
    for (Temp_tempList t = subset; t; t = t->tail)
    {
        int c = 0;
        for (Temp_tempList l = set; l; l = l->tail)
        {
            if (l->head == t->head)
            {
                BitArray_Set (r, c);
            }
            c++;
        }
    }
    return r;
}

Temp_tempList UnmapTempList (BitArray ba, Temp_tempList set)
{
    Temp_tempList r = NULL;
    int c = 0;
    for (; set; set = set->tail)
    {
        if (BitArray_IsSet (ba, c))
        {
            r = Temp_TempList (set->head, r);
        }
        c++;
    }

    return r;
}
