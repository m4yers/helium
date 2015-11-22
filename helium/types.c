#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "ext/util.h"
#include "ext/list.h"
#include "ext/mem.h"

#include "types.h"
#include "machine.h"

static struct Ty_ty_ tyauto = {Ty_auto, {"Auto"}, {NULL}};
Ty_ty Ty_Auto (void)
{
    return &tyauto;
}

// TODO allow to pass type under question, e.g. partially valid record
static struct Ty_ty_ tyinvalid = {Ty_invalid, {"Invalid"}, {NULL}};
Ty_ty Ty_Invalid (void)
{
    return &tyinvalid;
}

static struct Ty_ty_ tynil = {Ty_nil, {"Nil"}, {NULL}};
Ty_ty Ty_Nil (void)
{
    return &tynil;
}

static struct Ty_ty_ tyint = {Ty_int, {"Int"}, {NULL}};
Ty_ty Ty_Int (void)
{
    return &tyint;
}

static struct Ty_ty_ tystring = {Ty_string, {"String"}, {NULL}};
Ty_ty Ty_String (void)
{
    return &tystring;
}

static struct Ty_ty_ tyvoid = {Ty_void, {"Void"}, {NULL}};
Ty_ty Ty_Void (void)
{
    return &tyvoid;
}

Ty_ty Ty_Unknown (const char * name)
{
    Ty_ty p = checked_malloc (sizeof (*p));
    p->kind = Ty_unknown;
    p->meta.name = name;
    return &tyinvalid;
}

Ty_ty Ty_Record (Ty_fieldList fields)
{
    Ty_ty p = checked_malloc (sizeof (*p));
    p->kind = Ty_record;
    p->meta.name = "Record";
    p->u.record = fields;
    return p;
}

Ty_ty Ty_Array (Ty_ty type, int size)
{
    Ty_ty p = checked_malloc (sizeof (*p));
    p->kind = Ty_array;
    p->meta.name = "Array";
    p->u.array.type = type;
    p->u.array.size = size;
    return p;
}

Ty_ty Ty_Name (S_symbol sym, Ty_ty ty)
{
    Ty_ty p = checked_malloc (sizeof (*p));
    p->kind = Ty_name;
    p->meta.name = "Name";
    p->u.name.sym = sym;
    p->u.name.ty = ty;
    return p;
}

Ty_tyList Ty_TyList (Ty_ty head, Ty_tyList tail)
{
    Ty_tyList p = checked_malloc (sizeof (*p));
    p->head = head;
    p->tail = tail;
    return p;
}

Ty_field Ty_Field (S_symbol name, Ty_ty ty)
{
    Ty_field p = checked_malloc (sizeof (*p));
    p->name = name;
    p->ty = ty;
    return p;
}

Ty_fieldList Ty_FieldList (Ty_field head, Ty_fieldList tail)
{
    Ty_fieldList p = checked_malloc (sizeof (*p));
    p->head = head;
    p->tail = tail;
    return p;
}

// HMM alignment?
// HMM what with types smaller than word size?
// HMM what with types that do not evenly lay over array of words? e.g. array of bytes?
// HMM what with mixed types(records) where fields are smaller or bigger than word size? padding?
int Ty_SizeOf (Ty_ty type)
{
    switch (type->kind)
    {
    case Ty_int:
    {
        return F_wordSize;
    }
    case Ty_array:
    {
        return type->u.array.size * Ty_SizeOf (type->u.array.type);
    }
    case Ty_record:
    {
        int size = 0;
        LIST_FOREACH (field, type->u.record)
        {
            size += Ty_SizeOf (field->ty);
        }
        return size;
    }
    case Ty_name:
    {
        return Ty_SizeOf (type->u.name.ty);
    }
    default:
        return -1;
    }
}

/* printing functions - used for debugging */
static char str_ty[][12] =
{
    "ty_record", "ty_nil", "ty_int", "ty_string",
    "ty_array", "ty_name", "ty_void"
};

/* This will infinite loop on mutually recursive types */
void Ty_print (Ty_ty t)
{
    if (t == NULL)
    {
        printf ("null");
    }
    else
    {
        printf ("%s", str_ty[t->kind]);

        if (t->kind == Ty_name)
        {
            printf (", %s", S_Name (t->u.name.sym));
        }
    }
}

void TyList_print (Ty_tyList list)
{
    if (list == NULL)
    {
        printf ("null");
    }
    else
    {
        printf ("TyList( ");
        Ty_print (list->head);
        printf (", ");
        TyList_print (list->tail);
        printf (")");
    }
}
