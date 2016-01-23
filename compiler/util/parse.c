#include <stdlib.h>
#include <string.h>

#include "util/parse.h"

// TODO chars and length validation

intmax_t Parse_HexToInt (const char * str, Parse_status * status)
{
    *status = Parse_OK;

    size_t len = strlen(str);

    if (len == 0)
    {
        *status = Parse_EMPTY;
        return 0;
    }

    char * end;
    uintmax_t r = strtoll(str, &end, 16);

    if (end != (str + len))
    {
        *status = Parse_CHAR_EXCESS;
    }

    return r;
}

uintmax_t Parse_DecToUInt (const char * str, Parse_status * status)
{
    *status = Parse_OK;

    size_t len = strlen(str);

    if (len == 0)
    {
        *status = Parse_EMPTY;
        return 0;
    }

    char * end;
    uintmax_t r = strtoull(str, &end, 10);

    if (end != (str + len))
    {
        *status = Parse_CHAR_EXCESS;
    }

    return r;
}

intmax_t Parse_DecToInt (const char * str, Parse_status * status)
{
    *status = Parse_OK;

    size_t len = strlen(str);

    if (len == 0)
    {
        *status = Parse_EMPTY;
        return 0;
    }

    char * end;
    uintmax_t r = strtoll(str, &end, 10);

    if (end != (str + len))
    {
        *status = Parse_CHAR_EXCESS;
    }

    return r;
}

uintmax_t Parse_HexToUInt (const char * str, Parse_status * status)
{
    *status = Parse_OK;

    size_t len = strlen(str);

    if (len == 0)
    {
        *status = Parse_EMPTY;
        return 0;
    }

    char * end;
    uintmax_t r = strtoull(str, &end, 16);

    if (end != (str + len))
    {
        *status = Parse_CHAR_EXCESS;
    }

    return r;
}
