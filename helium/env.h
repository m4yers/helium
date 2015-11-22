#ifndef ENV_H_V0BEBNKU
#define ENV_H_V0BEBNKU

#include "types.h"
#include "symbol.h"
#include "translate.h"

// forward declaration
struct Semant_ContextType;

typedef struct Env_EnventryType
{
    enum { Env_varEntry, Env_funEntry } kind;

    union
    {
        struct
        {
            Tr_access access;
            Ty_ty ty;
        } var;

        struct
        {
            Tr_level level;
            Temp_label label;
            Ty_tyList formals;
            Ty_ty result;
        } fun;

    } u;

} * Env_Entry;

Env_Entry Env_VarEntryNew (Tr_access access, Ty_ty ty);
Env_Entry Env_FunEntryNew (Tr_level level, Temp_label label, Ty_tyList formals, Ty_ty result);

void Env_Init (struct Semant_ContextType * c);

#endif /* end of include guard: ENV_H_V0BEBNKU */
