#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "util/mem.h"
#include "util/str.h"

#include "symbol.h"
#include "frame.h"
#include "temp.h"
#include "asm.h"
#include "ir.h"

// TODO get rid of malloc

ASM_targets ASM_Targets (Temp_labelList labels)
{
    ASM_targets p = checked_malloc (sizeof * p);
    p->labels = labels;
    return p;
}

ASM_line ASM_Oper (const char * a, Temp_tempList d, Temp_tempList s, ASM_targets j)
{
    ASM_line p = (ASM_line) checked_malloc (sizeof * p);
    p->kind = I_OPER;
    p->u.OPER.assem = a;
    p->u.OPER.dst = d;
    p->u.OPER.src = s;
    p->u.OPER.jumps = j;
    return p;
}

ASM_line ASM_Label (const char * a, Temp_label label)
{
    ASM_line p = (ASM_line) checked_malloc (sizeof * p);
    p->kind = I_LABEL;
    p->u.LABEL.assem = a;
    p->u.LABEL.label = label;
    return p;
}

ASM_line ASM_Move (const char * a, Temp_tempList d, Temp_tempList s)
{
    ASM_line p = (ASM_line) checked_malloc (sizeof * p);
    p->kind = I_MOVE;
    p->u.MOVE.assem = a;
    p->u.MOVE.dst = d;
    p->u.MOVE.src = s;
    return p;
}

ASM_line ASM_MetaCallIn()
{
    ASM_line p = (ASM_line) checked_malloc (sizeof * p);
    p->kind = I_META;
    p->u.META.kind = I_META_CALL_IN;
    return p;
}

ASM_line ASM_MetaCallOut()
{
    ASM_line p = (ASM_line) checked_malloc (sizeof * p);
    p->kind = I_META;
    p->u.META.kind = I_META_CALL_OUT;
    return p;
}

ASM_line ASM_MetaCallComment (const char * text)
{
    ASM_line p = (ASM_line) checked_malloc (sizeof * p);
    p->kind = I_META;
    p->u.META.kind = I_META_COMMENT;
    p->u.META.COMMENT = text;
    return p;
}

ASM_lineList ASM_LineList (ASM_line head, ASM_lineList tail)
{
    ASM_lineList p = (ASM_lineList) checked_malloc (sizeof * p);
    p->head = head;
    p->tail = tail;
    return p;
}

ASM_lineList ASM_Splice (ASM_lineList a, ASM_lineList b)
{
    ASM_lineList p;
    if (a == NULL)
    {
        return b;
    }
    for (p = a; p->tail != NULL; p = p->tail) ;
    p->tail = b;
    return a;
}

static Temp_temp nthTemp (Temp_tempList list, int i)
{
    assert (list);
    if (i == 0)
    {
        return list->head;
    }
    else
    {
        return nthTemp (list->tail, i - 1);
    }
}

static Temp_label nthLabel (Temp_labelList list, int i)
{
    assert (list);
    if (i == 0)
    {
        return list->head;
    }
    else
    {
        return nthLabel (list->tail, i - 1);
    }
}

char * ASM_Format (
    char * result,
    const char * assem,
    Temp_tempList dst,
    Temp_tempList src,
    ASM_targets jumps,
    Temp_map m)
{
    /* strcpy(result, assem); */
    /* return result; */
    const char * p;
    int i = 0; /* offset to result const char * */
    for (p = assem; p && *p != '\0'; p++)
    {
        if (*p == '`')
        {
            switch (* (++p))
            {
            case 's':
            {
                int n = atoi (++p);
                const char * s = Temp_Look (m, nthTemp (src, n));
                strcpy(result + i, "$");
                i++;
                strcpy (result + i, s);
                i += strlen (s);
            }
            break;
            case 'd':
            {
                int n = atoi (++p);
                const char * s = Temp_Look (m, nthTemp (dst, n));
                strcpy(result + i, "$");
                i++;
                strcpy (result + i, s);
                i += strlen (s);
            }
            break;
            case 'j':
                assert (jumps);
                {
                    int n = atoi (++p);
                    const char * s = Temp_LabelString (nthLabel (jumps->labels, n));
                    strcpy (result + i, s);
                    i += strlen (s);
                }
                break;
            case '`':
                result[i] = '`';
                i++;
                break;
            default:
                assert (0);
            }
        }
        else
        {
            result[i] = *p;
            i++;
        }
    }
    result[i] = '\0';
    return result;
}

char * ASM_LineToString (ASM_line l, Temp_map m)
{
    char * r = checked_malloc (100);
    switch (l->kind)
    {
    case I_LABEL:
        return ASM_Format (r, l->u.LABEL.assem, NULL, NULL, NULL, m);
    case I_OPER:
        return ASM_Format (r, l->u.OPER.assem, l->u.OPER.dst, l->u.OPER.src, l->u.OPER.jumps, m);
    case I_MOVE:
        return ASM_Format (r, l->u.MOVE.assem, l->u.MOVE.dst, l->u.MOVE.src, NULL, m);
    case I_META:
        if (l->u.META.kind == I_META_COMMENT)
        {
            return strdup (l->u.META.COMMENT);
        }
    }
    return r;
}

char * ASM_LineListToString (ASM_lineList list, Temp_map m)
{
    struct String_t str = String ("");
    LIST_FOREACH (l, list)
    {
        const char * line = ASM_LineToString (l, m);
        String_Append (&str, line);
        String_Append (&str, '\n');
    }

    return str.data;
}

void ASM_PrintLine (FILE * out, ASM_line i, Temp_map m)
{
    char r[200];
    switch (i->kind)
    {
    case I_LABEL:
    {
        ASM_Format (r, i->u.LABEL.assem, NULL, NULL, NULL, m);
        fprintf (out, "%s\n", r);
        /* i->u.LABEL->label); */
        break;
    }
    case I_OPER:
    {
        if (strlen (i->u.OPER.assem) == 0)
        {
            break;
        }
        ASM_Format (r, i->u.OPER.assem, i->u.OPER.dst, i->u.OPER.src, i->u.OPER.jumps, m);
        fprintf (out, "  %s\n", r);
        break;
    }
    case I_MOVE:
    {
        ASM_Format (r, i->u.MOVE.assem, i->u.MOVE.dst, i->u.MOVE.src, NULL, m);
        fprintf (out, "  %s\n", r);
        break;
    }
    case I_META:
    {
        if (i->u.META.kind == I_META_COMMENT)
        {
            fprintf (out, ";%s\n", i->u.META.COMMENT);
        }
    }
    }
}
