#ifndef ENV_H_V0BEBNKU
#define ENV_H_V0BEBNKU

#include "types.h"
#include "symbol.h"
#include "translate.h"

// forward declaration
struct Semant_ContextType;

struct Env_funEntry_t
{
    Tr_level level;
    Temp_label label;
    S_symbolList names;
    Ty_tyList types;
    struct Vector_t /** of struct Pair_t */ formals;
    Ty_ty result;
};

struct Env_varEntry_t
{
    Tr_access access;
    Ty_ty ty;
};

typedef struct Env_Entry_t
{
    enum { Env_varEntry, Env_funEntry } kind;

    Tr_level level;

    union
    {
        struct Env_varEntry_t var;
        struct Env_funEntry_t fun;
    } u;

} * Env_Entry;

Env_Entry Env_VarEntryNew (Tr_level level, Tr_access access, Ty_ty ty);
Env_Entry Env_FunEntryNew (Tr_level parent, Tr_level level, Temp_label label, S_symbolList names, Ty_tyList types, Ty_ty result);

void Env_Init (struct Semant_ContextType * c);

#endif /* end of include guard: ENV_H_V0BEBNKU */
