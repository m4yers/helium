#include "ext/util.h"
#include "ext/bool.h"

extern bool EM_anyErrors;

extern int EM_tokPos;

// TODO remove this
void EM_newline (void);

void EM_enable (void);
void EM_disable (void);
void EM_error (int, const char *, ...);
void EM_impossible (string, ...);
void EM_reset (const char * filename);
