#include "ext/mem.h"

#include "env.h"
#include "semant.h"

Env_Entry Env_VarEntryNew (Tr_access access, Ty_ty ty)
{
    Env_Entry e = checked_malloc (sizeof (*e));
    e->kind = Env_varEntry;
    e->u.var.access = access;
    e->u.var.ty = ty;
    return e;
}

Env_Entry Env_FunEntryNew (Tr_level level, Temp_label label, Ty_tyList formals, Ty_ty result)
{
    Env_Entry e = checked_malloc (sizeof (*e));
    e->kind = Env_funEntry;
    e->u.fun.level = level;
    e->u.fun.label = label;
    e->u.fun.formals = formals;
    e->u.fun.result = result;
    return e;
}

void Env_Init (Semant_Context c)
{
    S_table tenv = S_Empty();

    S_Enter (tenv, S_Symbol ("void"), Ty_Void());
    S_Enter (tenv, S_Symbol ("nil"), Ty_Nil());
    S_Enter (tenv, S_Symbol ("int"), Ty_Int());
    S_Enter (tenv, S_Symbol ("string"), Ty_String());

    c->tenv = tenv;
    c->venv = S_Empty();
}
