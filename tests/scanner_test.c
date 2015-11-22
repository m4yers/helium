#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

typedef struct test_case_part test_case_part;
struct test_case_part
{
    char * str;
    int len;
};

typedef struct test_case test_case;
struct test_case
{
    test_case_part code;
    test_case_part tokens;
    test_case_part ast;
};

static test_case * read_test_case (const char * filename)
{
    const int buffer = 512;

    FILE * file = fopen (filename, "r");

    if (file)
    {
        test_case * result = malloc (sizeof (test_case));
        test_case_part * part = (test_case_part *) result;
        part->str = malloc (buffer);
        part->len = 0;
        int is_code = 1;
        int c;

        while ((c = getc (file)) != EOF)
        {
            if (c == '#')
            {
                part++;
                part->str = malloc (buffer);
                part->len = 0;
                is_code = 0;
                continue;
            }

            //
            // Drop unneeded newlines
            if (!is_code && c == '\n')
            {
                continue;
            }

            if (part->len % buffer == 0 && part->len != 0)
            {
                part->str = realloc (part->str, part->len + buffer);
            }

            part->str[part->len++] = (char) c;
        }

        fclose (file);

        return result;
    }
    else
    {
        fprintf (stderr, "Cannot read file '%s'\n", filename);
    }

    return NULL;
}

static void scanner_test (void ** state)
{
    //yyin = fopen (fname, "r");
    test_case * test = read_test_case ("../../../tests/cases/test1.tig");

    //printf ("code.len: %d\n", test->code.len);
    //printf ("code.str: \n");
    //fwrite (test->code.str, test->code.len, 1, stdout);

    //printf ("tokens.len: %d\n", test->tokens.len);
    //printf ("tokens.str: ");
    //fwrite (test->tokens.str, test->tokens.len, 1, stdout);
    //printf ("\n");

    //printf ("ast.len: %d\n", test->ast.len);
    //printf ("ast.str: ");
    //fwrite (test->ast.str, test->ast.len, 1, stdout);
    //printf ("\n");

    free (test);

    (void) state; /* unused */
}

int main (void)
{
    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test (scanner_test),
    };
    return cmocka_run_group_tests (tests, NULL, NULL);
}
