#ifndef PROGRAM_H_NTR7UGNX
#define PROGRAM_H_NTR7UGNX

#include "ext/vector.h"

#include "frame.h"
#include "ast.h"

typedef struct Program_Module_
{
    const char * input;
    A_decList ast;

    struct
    {
        F_fragList strings;
        F_fragList functions;
    } fragments;

    struct
    {
        Vector /* struct Error */ lexer;
        Vector /* struct Error */ parser;
        Vector /* struct Error */ semant;
    } errors;

    Vector /* of RA_Result */ results;

} * Program_Module;

Program_Module Program_ModuleNew (void);
void Program_ParseArguments (Program_Module m, int argc, char ** argv);
void Program_AddFragment (Program_Module p, F_frag f);
void Program_PrintAssembly (FILE * file, Program_Module p);

#endif /* end of include guard: PROGRAM_H_NTR7UGNX */
