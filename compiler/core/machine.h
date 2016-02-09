#ifndef MACHINE_H_EASKR5CA
#define MACHINE_H_EASKR5CA

#include "util/list.h"
#include "util/str.h"
#include "util/bool.h"

#include "core/temp.h"

typedef struct M_opCode_t
{
    const struct String_t name;
    const struct String_t format;
} * M_opCode;

#define M_OpCode(c,f) { .name = String(c), .format = String(f) }

extern const int M_wordSize;

extern Temp_map regs_map;

typedef struct
{
    int number;
    Temp_tempList temps, last_temp;
    U_stringList names, last_name;
} * M_regs;

extern M_regs M_regs_all;

M_regs M_Regs (Temp_tempList temps, U_stringList names);

Temp_temp M_RegGet_s (M_regs regs, const char * name);
Temp_temp M_RegGet_u (M_regs regs, int index);

#define M_RegGet(a,b)                                                                   \
    _Generic ((0,b),                                                                    \
            const char *: M_RegGet_s,                                                   \
            char *: M_RegGet_s,                                                         \
            int: M_RegGet_u                                                             \
            )(a,b)

const char * M_RegGetName (M_regs regs, int index);

bool M_RegsHas (M_regs regs, Temp_temp reg);
void M_RegsAdd (M_regs regs, Temp_temp temp, const char * name);
Temp_map M_RegsToTempMap (Temp_map map, M_regs regs);

#endif /* end of include guard: MACHINE_H_EASKR5CA */
