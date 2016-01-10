#include "ext/mem.h"

#include "env.h"
#include "semant.h"

Env_Entry Env_VarEntryNew (Tr_level level, Tr_access access, Ty_ty ty)
{
    Env_Entry e = checked_malloc (sizeof (*e));
    e->kind = Env_varEntry;
    e->level = level;
    e->u.var.access = access;
    e->u.var.ty = ty;
    return e;
}

Env_Entry Env_FunEntryNew (Tr_level parent, Tr_level level, Temp_label label, S_symbolList names, Ty_tyList types, Ty_ty result)
{
    Env_Entry e = checked_malloc (sizeof (*e));
    e->kind = Env_funEntry;
    e->level = parent;
    e->u.fun.level = level;
    e->u.fun.label = label;
    e->u.fun.names = names;
    e->u.fun.types = types;
    e->u.fun.result = result;
    return e;
}

void Env_Init (Sema_Context c)
{
    S_table tenv = S_Empty();

    S_Enter (tenv, S_Symbol ("void"), Ty_Void());
    S_Enter (tenv, S_Symbol ("nil"), Ty_Nil());
    S_Enter (tenv, S_Symbol ("int"), Ty_Int());
    S_Enter (tenv, S_Symbol ("str"), Ty_Str());
    S_Enter (tenv, S_Symbol ("&str"), Ty_Pointer (Ty_Str()));

    c->tenv = tenv;
    c->venv = S_Empty();
}
