#include <argp.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "util/vector.h"
#include "util/list.h"
#include "util/util.h"
#include "util/bool.h"
#include "util/mem.h"
#include "util/str.h"

#include "core/program.h"
#include "core/regalloc.h"
#include "core/machine.h"
#include "core/error.h"
#include "core/frame.h"
#include "core/asm.h"

Program_Module Program_ModuleNew()
{
    Program_Module r = checked_malloc (sizeof * r);

    r->options.file = NULL;
    r->options.output = NULL;
    Vector_Init (&r->options.debug, struct Program_Option_t);

    r->ast = NULL;
    r->fragments.strings = NULL;
    r->fragments.functions = NULL;
    r->fragments.code = NULL;

    Vector_Init (&r->errors.lexer, struct Error_t);
    Vector_Init (&r->errors.parser, struct Error_t);
    Vector_Init (&r->errors.preproc, struct Error_t);
    Vector_Init (&r->errors.semant, struct Error_t);

    Vector_Init (&r->results.functions, RA_Result);
    Vector_Init (&r->results.code, ASM_lineList);

    return r;
}

/*********************************************************************
 *                         Arguments parsing                         *
 *********************************************************************/

#ifdef CONFIG_BUILD_EXE
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
    {
        .name = 0,
        .key = 'o',
        .arg = "FILENAME",
        .group = 0,
        .flags = OPTION_ARG_OPTIONAL,
        .doc = "Write output to FILENAME"
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
    case 'o':
    {
        m->options.output = String_New (arg);
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

    argp_parse (&argp, argc, argv, 0, 0, &m->options);
}
#endif


/*********************************************************************
 *                             Fragments                             *
 *********************************************************************/

Temp_label Program_AddStringFrag(Program_Module p, const char * str, F_stringType type)
{
    Temp_label rslt = NULL;
    /*
     * We traverse the list of string fragments in hope to find a string exactly the same we've
     * been passed, thus we have a chance to "merge" these fragments into using the same data
     * declaration and thus the same label.
     */
    LIST_FOREACH (f, p->fragments.strings)
    {
        if (strcmp(str, f->u.str.str) == 0 && type == f->u.str.type)
        {
            rslt = f->u.str.label;
            break;
        }
    }

    if (!rslt)
    {
        U_Create (F_frag, frag)
        {
            .kind = F_stringFrag,
            .u.str.str = str,
            .u.str.type = type,
            .u.str.label = Temp_NewLabel()
        };
        LIST_PUSH (p->fragments.strings, frag);

        rslt = frag->u.str.label;
    }

    return rslt;
}

void Program_AddFragment (Program_Module p, F_frag f)
{
    assert (p);
    assert (f);

    switch (f->kind)
    {
    case F_procFrag:
        LIST_PUSH (p->fragments.functions, f);
        break;

    case F_codeFrag:
        LIST_PUSH (p->fragments.code, f);
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
            switch (fragment->u.str.type)
            {
            /*
             * NOTE string length endianness matters because it will be read with word length
             * instruction, but the string bytes order MUST be in the direct order as written in
             * spoken language, because it will be read byte by byte and endianness is of no
             * concern here
             */
            case F_lps:
            {
                fprintf (file, "%s: .byte", fragment->u.str.label->name);

                // TODO check for 32-bit overflow?
                // TODO little-endian vs big-endian 'if'?
                int size = (int)strlen (fragment->u.str.str);
                fprintf (file, " 0x%02x", (size >> 0x00) & 0xFF);
                fprintf (file, ",0x%02x", (size >> 0x08) & 0xFF);
                fprintf (file, ",0x%02x", (size >> 0x10) & 0xFF);
                fprintf (file, ",0x%02x", (size >> 0x18) & 0xFF);

                const char * str = fragment->u.str.str;

                while (*str)
                {
                    fprintf (file, ",'%c'", *str);
                    str++;
                }

                fprintf (file, "\n");

                break;
            }
            case F_sz:
            {
                fprintf (file,
                         "%s: .asciiz \"%s\"\n",
                         fragment->u.str.label->name,
                         fragment->u.str.str);
                break;
            }
            }
        }
        fprintf (file, "\n");
    }
    fprintf (file, ".text\n");
    fprintf (file, ".globl main\n");

    VECTOR_FOREACH (ASM_lineList, ll, &p->results.code)
    {
        LIST_FOREACH (line, *ll)
        {
            if (line->kind != I_META)
            {
                ASM_PrintLine (file, line, regs_map);
            }
        }
    }

    VECTOR_FOREACH (RA_Result, r, &p->results.functions)
    {
        LIST_FOREACH (line, (*r)->il)
        {
            if (line->kind != I_META)
            {
                /* ASM_PrintLine (file, line, Temp_Name()); */
                ASM_PrintLine (file, line, Temp_LayerMap ((*r)->coloring, Temp_Name()));
            }
        }
    }
}
