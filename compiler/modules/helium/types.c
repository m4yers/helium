#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "util/util.h"
#include "util/list.h"
#include "util/mem.h"

#include "core/machine.h"

#include "modules/helium/types.h"

static struct Ty_ty_ tyauto =
{
    .kind = Ty_auto,
    .meta = {
        .name = "auto",
        .is_internal = TRUE,
        .is_pointer = FALSE,
        .is_handle = FALSE
    },
    .u = {NULL}
};
Ty_ty Ty_Auto (void)
{
    return &tyauto;
}

static struct Ty_ty_ tyinvalid =
{
    .kind = Ty_invalid,
    .meta = {
        .name = "invalid",
        .is_internal = TRUE,
        .is_pointer = FALSE,
        .is_handle = FALSE
    },
    .u = {NULL}
};
Ty_ty Ty_Invalid (void)
{
    return &tyinvalid;
}

static struct Ty_ty_ tynil =
{
    .kind = Ty_nil,
    .meta = {
        .name = "nil",
        .is_internal = FALSE,
        .is_pointer = TRUE,
        .is_handle = FALSE
    },
    .u = {NULL}
};
Ty_ty Ty_Nil (void)
{
    return &tynil;
}

static struct Ty_ty_ tyint =
{
    .kind = Ty_int,
    .meta = {
        .name = "int",
        .is_internal = FALSE,
        .is_pointer = FALSE,
        .is_handle = FALSE
    },
    .u = {NULL}
};
Ty_ty Ty_Int (void)
{
    return &tyint;
}

static struct Ty_ty_ tystr =
{
    .kind = Ty_string,
    .meta = {
        .name = "str",
        .is_internal = FALSE,
        .is_pointer = FALSE,
        .is_handle = FALSE
    },
    .u = {NULL}
};
Ty_ty Ty_Str (void)
{
    return &tystr;
}

static struct Ty_ty_ tyvoid =
{
    .kind = Ty_void,
    .meta = {
        .name = "void",
        .is_internal = FALSE,
        .is_pointer = FALSE,
        .is_handle = FALSE
    },
    .u = {NULL}
};
Ty_ty Ty_Void (void)
{
    return &tyvoid;
}

Ty_ty Ty_Pointer (Ty_ty type)
{
    Ty_ty p = checked_malloc (sizeof (*p));
    p->kind = Ty_pointer;
    p->meta.name = "pointer";
    p->meta.is_internal = FALSE;
    p->meta.is_pointer = TRUE;
    p->meta.is_handle = FALSE;
    p->u.pointer = type;
    return p;
}

Ty_ty Ty_Record (Ty_fieldList fields)
{
    Ty_ty p = checked_malloc (sizeof (*p));
    p->kind = Ty_record;
    p->meta.name = "record";
    p->meta.is_internal = FALSE;
    p->meta.is_pointer = FALSE;
    p->meta.is_handle = TRUE;
    p->u.record = fields;
    return p;
}

Ty_ty Ty_Array (Ty_ty type, int size)
{
    Ty_ty p = checked_malloc (sizeof (*p));
    p->kind = Ty_array;
    p->meta.name = "array";
    p->meta.is_internal = FALSE;
    p->meta.is_pointer = FALSE;
    p->meta.is_handle = TRUE;
    p->u.array.type = type;
    p->u.array.size = size;
    return p;
}

Ty_ty Ty_Name (S_symbol sym, Ty_ty ty)
{
    Ty_ty p = checked_malloc (sizeof (*p));
    p->kind = Ty_name;
    p->meta.name = "name";
    p->meta.is_internal = FALSE;
    p->meta.is_pointer = FALSE;
    p->meta.is_handle = FALSE;
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

Ty_ty GetActualType (Ty_ty ty)
{
    if (ty->kind == Ty_name)
    {
        if (ty->u.name.ty == NULL)
        {
            printf ("NULL");
        }
        return ty->u.name.ty;
    }
    return ty;
}

String GetQTypeName (Ty_ty ty, String str)
{
    if (!str)
    {
        str = String_New ("");
    }

    switch (ty->kind)
    {
    case Ty_array:
    {
        String_Append (str, ty->meta.name);
        String_Append (str, "[");
        GetQTypeName (ty->u.array.type, str);
        String_Append (str, ",");
        String_Append (str, ty->u.array.size);
        String_Append (str, "]");
        break;
    }
    case Ty_record:
    {
        String_Append (str, ty->meta.name);
        String_Append (str, "{");
        LIST_FOREACH (f, ty->u.record)
        {
            String_Append (str, f->name->name);
            String_Append (str, ":");
            GetQTypeName (f->ty, str);
            String_Append (str, ",");
        }
        String_Append (str, "}");
        break;
    }
    case Ty_name:
    {
        String_Append (str, ty->u.name.sym->name);
        String_Append (str, "__");
        GetQTypeName (ty->u.name.ty, str);
        break;
    }
    case Ty_pointer:
    {
        String_Append (str, "&");
        GetQTypeName (ty->u.pointer, str);
        break;
    }
    default:
    {
        String_Append (str, ty->meta.name);
        break;
    }
    }
    return str;
}

// HMM alignment?
// HMM what with types smaller than word size?
// HMM what with types that do not evenly lay over array of words? e.g. array of bytes?
// HMM what with mixed types(records) where fields are smaller or bigger than word size? padding?
int Ty_SizeOf (Ty_ty type)
{
    switch (type->kind)
    {
    case Ty_pointer:
    case Ty_int:
    {
        return M_wordSize;
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
        // HMM... is this correct behaviour?
        return -1;
    }
}
