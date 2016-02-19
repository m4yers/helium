#ifndef ERROR_H_AVNIWZ7O
#define ERROR_H_AVNIWZ7O

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "core/ast.h"
#include "util/list.h"

// TODO make it use String

typedef struct Error_t
{
    struct A_loc_t loc;
    int code;
    const char * text;
} * Error;

static inline Error Error_NewPtr (const A_loc loc, int code, const char * format, ...)
{
    char * buffer = checked_malloc (1024);  // FIXME remove hardocded size?

    va_list ap;
    va_start (ap, format);
    vsprintf (buffer, format, ap);
    va_end (ap);

    U_Create (Error, e)
    {
        .loc = *loc,
        .code = code,
        .text = buffer
    };

    return e;
}

LIST_DEFINE (ErrorList, Error)

extern struct Error_t Error_OK;

struct Error_t Error_New (const A_loc loc, int code, const char * format, ...);
char * Error_ToString (Error err);
void Error_Print (FILE * out, Error err);

#endif /* end of include guard: ERROR_H_AVNIWZ7O */
