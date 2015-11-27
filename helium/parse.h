#ifndef PARSE_H_L51RYZMQ
#define PARSE_H_L51RYZMQ

#include "ext/str.h"

#include "ast.h"
#include "program.h"

int Parse_File (Program_Module m, String filename);
int Parse_String (Program_Module m, const char * s);

#endif /* end of include guard: PARSE_H_L51RYZMQ */

