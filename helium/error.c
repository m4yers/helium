#include <stdarg.h>
#include "string.h"

#include "ext/mem.h"

#include "error.h"

struct Error Error_New (const A_loc loc, int code, const char * format, ...)
{
    char * buffer = checked_malloc (1024);

    va_list ap;
    va_start (ap, format);
    vsprintf (buffer, format, ap);
    va_end (ap);

    struct Error e = { .loc = *loc, .code = code, .text = buffer };
    return e;
}

char * Error_ToString (struct Error * err)
{
    char * buffer = checked_malloc (strlen (err->text) + 20);
    sprintf (buffer, "%d,%d %d %s",
             err->loc.first_line,
             err->loc.first_column,
             err->code,
             err->text);
    return buffer;
}

void Error_Print (FILE * out, struct Error * err)
{
    fprintf (out, "ERROR %d,%d %d %s\n",
             err->loc.first_line,
             err->loc.first_column,
             err->code,
             err->text);
}
