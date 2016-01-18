#ifndef PARSE_H_L51RYZMQ
#define PARSE_H_L51RYZMQ

#include "util/str.h"

#include "core/program.h"
#include "core/ast.h"

int Parse_File (Program_Module m, String filename);
int Parse_String (Program_Module m, const char * s);

A_asmStmList ParseAsm(A_loc loc, const char * input);

#endif /* end of include guard: PARSE_H_L51RYZMQ */

