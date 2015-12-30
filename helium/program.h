#ifndef PROGRAM_H_NTR7UGNX
#define PROGRAM_H_NTR7UGNX

#include "ext/vector.h"
#include "ext/str.h"
#include "ext/list.h"

#include "frame.h"
#include "ast_helium.h"

struct Program_Option_t
{
    struct String_t key;
    struct String_t value;
};

struct Program_Options_t
{
    String file;
    String output;
    struct Vector_t /** Program_Option_t */ debug;
};

typedef struct Program_Module_
{
    struct Program_Options_t options;

    A_decList ast;

    struct
    {
        F_fragList strings;
        F_fragList functions;
    } fragments;

    struct
    {
        struct Vector_t /* struct Error */ lexer;
        struct Vector_t /* struct Error */ parser;
        struct Vector_t /* struct Error */ preproc;
        struct Vector_t /* struct Error */ semant;
    } errors;

    struct Vector_t /* of RA_Result */ results;

} * Program_Module;

Program_Module Program_ModuleNew (void);
void Program_ParseArguments (Program_Module m, int argc, char ** argv);
void Program_AddFragment (Program_Module p, F_frag f);
void Program_PrintAssembly (FILE * file, Program_Module p);

#endif /* end of include guard: PROGRAM_H_NTR7UGNX */
