#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "ext/util.h"
#include "ext/bool.h"
#include "ext/mem.h"

#include "errormsg.h"

// TODO make it 0 - 3
static bool enabled = TRUE;

void EM_enable (void)
{
    enabled = TRUE;
}

void EM_disable (void)
{
    enabled = FALSE;
}

bool anyErrors = FALSE;

static const char * fileName = NULL;

static int lineNum = 1;

int EM_tokPos = 0;

typedef struct intList
{
    int i;
    struct intList * rest;
} * IntList;

static IntList intList (int i, IntList rest)
{
    IntList l = checked_malloc (sizeof * l);
    l->i = i;
    l->rest = rest;
    return l;
}

static IntList linePos = NULL;

void EM_newline (void)
{
    lineNum++;
    linePos = intList (EM_tokPos, linePos);
}

void EM_error (int pos, const char * message, ...)
{
    if (!enabled)
    {
        return;
    }

    va_list ap;
    IntList lines = linePos;
    int num = lineNum;


    anyErrors = TRUE;

    while (lines && lines->i >= pos)
    {
        lines = lines->rest;
        num--;
    }

    if (fileName)
    {
        fprintf (stderr, "%s:", fileName);
    }

    if (lines)
    {
        fprintf (stderr, "%d.%d: ", num, pos - lines->i);
    }

    va_start (ap, message);
    vfprintf (stderr, message, ap);
    va_end (ap);
    fprintf (stderr, "\n");

}

void EM_reset (const char * fname)
{
    anyErrors = FALSE;
    fileName = fname;
    lineNum = 1;
    linePos = intList (0, NULL);
}
