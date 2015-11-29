#ifndef MACHINE_H_EASKR5CA
#define MACHINE_H_EASKR5CA

#include "ext/list.h"
#include "temp.h"

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

bool F_RegistersContains (F_registers regs, Temp_temp reg);
void F_RegistersAdd (F_registers regs, Temp_temp temp, const char * name);
Temp_map F_RegistersToMap (Temp_map map, F_registers regs);


#endif /* end of include guard: MACHINE_H_EASKR5CA */
