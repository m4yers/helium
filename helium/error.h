#ifndef ERROR_H_AVNIWZ7O
#define ERROR_H_AVNIWZ7O

#include "ast_helium.h"

struct Error
{
    struct A_loc_t loc;
    int code;
    const char * text;
};

struct Error Error_New (const A_loc loc, int code, const char * format, ...);
char * Error_ToString (struct Error * err);
void Error_Print (FILE * out, struct Error * err);

#endif /* end of include guard: ERROR_H_AVNIWZ7O */
