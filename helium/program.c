#include <argp.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ext/vector.h"
#include "ext/list.h"
#include "ext/util.h"
#include "ext/mem.h"

#include "program.h"
#include "error.h"
#include "regalloc.h"

Program_Module Program_ModuleNew()
{
    Program_Module r = checked_malloc (sizeof * r);

    r->input = NULL;
    r->ast = NULL;
    r->fragments.strings = NULL;
    r->fragments.functions = NULL;

    Vector_Init (&r->errors.lexer, struct Error);
    Vector_Init (&r->errors.parser, struct Error);
    Vector_Init (&r->errors.semant, struct Error);

    Vector_Init (&r->results, RA_Result);

    return r;
}

/*********************************************************************
 *                         Arguments parsing                         *
 *********************************************************************/

const char * argp_program_version = "helium-0.0.1";
const char * argp_program_bug_address = "<m4yers@gmail.com>";
static char doc[] = "Helium is a system language and compiler. Work in progress...";
static char args_doc[] = "FILENAME";
static struct argp_option options[] =
{
    {
        "verbose", 'v', "LEVEL", OPTION_ARG_OPTIONAL,
        "Verbosity level. Default is 3.", 0
    },
    {
        0, 'Z', "FLAG", OPTION_ARG_OPTIONAL,
        "Internal debug options Use -Z help to print available options.", 0
    }
};

static struct argp argp = { 0, 0, 0, doc, 0, 0, 0 };

/* static Program_Option Program_OptionNew (const char * key, const char * value) */
/* { */
/*     Program_Option r = checked_malloc (sizeof (*r)); */
/*     r->key = key; */
/*     r->key_len = 0; */
/*     r->value = value; */
/*     r->value_len = 0; */
/*     return r; */
/* } */

/* static Program_OptionList ParseArguments (const char * args) */
/* { */
/* Program_OptionList list = NULL; */
/* Program_Option opt = Program_OptionNew (NULL, NULL); */
/*  */
/* bool isKey = TRUE; */
/*  */
/* for (; *args != '\n'; args++) */
/* { */
/*     char token = * args; */
/*     switch (token) */
/*     { */
/*     case ',': */
/*     { */
/*         LIST_PUSH (list, opt); */
/*         opt = Program_OptionNew (NULL, NULL); */
/*         isKey = TRUE; */
/*         break; */
/*     } */
/*     case '=': */
/*     { */
/*         isKey = FALSE; */
/*         break; */
/*     } */
/*     default: */
/*     { */
/*         if (isKey) */
/*         { */
/*             if (opt->key == NULL) */
/*             { */
/*  */
/*             } */
/*         } */
/*         break; */
/*     } */
/*     } */
/* } */
/*  */
/* return list; */
/* } */

static error_t parse_opt (int key, char * arg, struct argp_state * state)
{
    Program_Module m = state->input;

    switch (key)
    {
    case 'Z':
    {
        /* m->options.debug = ParseArguments (arg); */
        break;
    }
    }
}

void Program_ParseArguments (Program_Module m, int argc, char ** argv)
{
    (void) m;
    (void) argc;
    (void) argv;

#ifdef CONFIG_BUILD_EXE
    argp_parse (&argp, argc, argv, 0, 0, &m->options);
#endif
}


/*********************************************************************
 *                             Fragments                             *
 *********************************************************************/

void Program_AddFragment (Program_Module p, F_frag f)
{
    assert (p);
    assert (f);

    switch (f->kind)
    {
    case F_stringFrag:
        LIST_PUSH (p->fragments.strings, f);
        break;

    case F_procFrag:
        LIST_PUSH (p->fragments.functions, f);
        break;

    default:
        assert (0);
        break;
    }
}


/*********************************************************************
 *                             Printing                              *
 *********************************************************************/

void Program_PrintAssembly (FILE * file, Program_Module p)
{
    assert (file);
    assert (p);

    if (p->fragments.strings)
    {
        fprintf (file, ".data\n");
        LIST_FOREACH (fragment, p->fragments.strings)
        {
            fprintf (file,
                     "%s: .asciiz \"%s\"\n",
                     fragment->u.str.label->name,
                     fragment->u.str.str);
        }
        fprintf (file, "\n");
    }
    fprintf (file, ".text\n");
    fprintf (file, ".globl main\n");
    VECTOR_FOREACH (RA_Result, r, &p->results)
    {
        LIST_FOREACH (line, (*r)->il)
        {
            if (line->kind != I_META)
            {
                ASM_PrintLine (file, line, Temp_LayerMap ((*r)->coloring, Temp_Name()));
            }
        }
    }
}
