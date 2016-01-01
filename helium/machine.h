#ifndef MACHINE_H_EASKR5CA
#define MACHINE_H_EASKR5CA

#include "ext/list.h"
#include "ext/str.h"
#include "ext/bool.h"

#include "temp.h"

typedef struct M_opCode_t
{
    const struct String_t name;
    const struct String_t format;
} * M_opCode;

#define M_OpCode(c,f) { .name = String(c), .format = String(f) }

extern const int F_wordSize;

typedef struct
{
    int number;
    Temp_tempList temps, last_temp;
    U_stringList names, last_name;
} * F_registers;

F_registers F_Registers (Temp_tempList temps, U_stringList names);

// TODO make these generic
Temp_temp F_RegistersGet (F_registers regs, int index);
Temp_temp F_RegistersGet_s (F_registers regs, const char * name);

const char * F_RegistersGetName (F_registers regs, int index);

bool F_RegistersContains (F_registers regs, Temp_temp reg);
void F_RegistersAdd (F_registers regs, Temp_temp temp, const char * name);
Temp_map F_RegistersToMap (Temp_map map, F_registers regs);


#endif /* end of include guard: MACHINE_H_EASKR5CA */
