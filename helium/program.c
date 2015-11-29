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

    r->options.file = NULL;
    Vector_Init (&r->options.debug, struct Program_Option_t);

    r->ast = NULL;
    r->fragments.strings = NULL;
    r->fragments.functions = NULL;

    Vector_Init (&r->errors.lexer, struct Error);
    Vector_Init (&r->errors.parser, struct Error);
    Vector_Init (&r->errors.preproc, struct Error);
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
        .name = "verbose",
        .key = 'v',
        .arg = "LEVEL",
        .group = 0,
        .flags = OPTION_ARG_OPTIONAL,
        .doc = "Verbosity level. Default is 3."
    },
    {
        .name = 0,
        .key = 'Z',
        .arg = "OPTIONS",
        .group = 0,
        .flags = OPTION_ARG_OPTIONAL,
        .doc = "Internal debug options Use -Z help to print available options."
    },
    {0, 0, 0, 0, 0, 0}
};

static void ParseArguments (Vector /** struct Program_Option_t */ v, const char * args)
{
    struct Program_Option_t opt;
    String_Init (&opt.key, "");
    String_Init (&opt.value, "");

    bool isKey = TRUE;

    for (; *args != '\0'; args++)
    {
        char token = *args;
        switch (token)
        {
        case ',':
        {
            Vector_Push (v, &opt);
            String_Init (&opt.key, "");
            String_Init (&opt.value, "");
            isKey = TRUE;
            break;
        }
        case '=':
        {
            isKey = FALSE;
            break;
        }
        default:
        {
            if (isKey)
            {
                String_PushBack (&opt.key, token);
            }
            else
            {
                String_PushBack (&opt.value, token);
            }
            break;
        }
        }
    }

    if (!String_Empty (&opt.key) || !String_Empty (&opt.value))
    {
        Vector_Push (v, &opt);
    }
}

static error_t parse_opt (int key, char * arg, struct argp_state * state)
{
    Program_Module m = state->input;

    switch (key)
    {
    case 'Z':
    {
        ParseArguments (&m->options.debug, arg);
        break;
    }
    case ARGP_KEY_ARG:
    {
        if (state->arg_num > 1)
        {
            argp_usage (state);
            break;
        }

        m->options.file = String_New (arg);
        break;
    }
    case ARGP_KEY_END:
    {
        if (m->options.file == NULL)
        {
            printf ("You must provide a file to compile.\n\n");
            argp_usage (state);
        }
        break;
    }
    default:
    {
        return ARGP_ERR_UNKNOWN;
    }
    }

    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

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
