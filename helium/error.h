#ifndef ERROR_H_AVNIWZ7O
#define ERROR_H_AVNIWZ7O

#include "ast_helium.h"

// TODO make it use String

typedef struct Error_t
{
    struct A_loc_t loc;
    int code;
    const char * text;
} * Error;

extern struct Error_t Error_OK;

struct Error_t Error_New (const A_loc loc, int code, const char * format, ...);
char * Error_ToString (Error err);
void Error_Print (FILE * out, Error err);

#endif /* end of include guard: ERROR_H_AVNIWZ7O */
