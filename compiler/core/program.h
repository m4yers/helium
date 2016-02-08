#ifndef PROGRAM_H_NTR7UGNX
#define PROGRAM_H_NTR7UGNX

#include "util/vector.h"
#include "util/str.h"
#include "util/list.h"

#include "modules/helium/ast.h"
#include "core/frame.h"

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
        F_fragList code;
    } fragments;

    struct
    {
        struct Vector_t /* struct Error_t */ lexer;
        struct Vector_t /* struct Error_t */ parser;
        struct Vector_t /* struct Error_t */ preproc;
        struct Vector_t /* struct Error_t */ semant;
    } errors;

    struct
    {
        struct Vector_t /* of RA_Result    */ functions;
        struct Vector_t /* of ASM_lineList */ code;
    } results;

} * Program_Module;

Program_Module Program_ModuleNew (void);
void Program_ParseArguments (Program_Module m, int argc, char ** argv);
Temp_label Program_AddStringFrag(Program_Module p, const char * str, F_stringType type);
void Program_AddFragment (Program_Module p, F_frag f);
void Program_PrintAssembly (FILE * file, Program_Module p);

#endif /* end of include guard: PROGRAM_H_NTR7UGNX */
