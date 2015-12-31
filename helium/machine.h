#ifndef MACHINE_H_EASKR5CA
#define MACHINE_H_EASKR5CA

#include "ext/list.h"
#include "ext/str.h"
#include "ext/bool.h"

#include "temp.h"

typedef enum
{
    M_opCode_R, M_opCode_I, M_opCode_J, M_opCode_CI
} M_opCodeKind;

struct M_opCode_t
{
    struct String_t code;
    M_opCodeKind kind;
} * M_opCode;

#define M_OpCode(c,k) { .code = String(c), .kind = k }

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
