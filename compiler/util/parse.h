#pragma once

#include <stdint.h>

typedef enum
{
    Parse_OK,
    Parse_EMPTY,
    Parse_CHAR_EXCESS,
} Parse_status;

intmax_t Parse_HexToInt (const char * str, Parse_status * status);
uintmax_t Parse_HexToUInt (const char * str, Parse_status * status);
intmax_t Parse_DecToInt (const char * str, Parse_status * status);
uintmax_t Parse_DecToUInt (const char * str, Parse_status * status);
