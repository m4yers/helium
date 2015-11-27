#ifndef SYMBOL_H_A9CT48M3
#define SYMBOL_H_A9CT48M3

typedef struct S_symbol_ * S_symbol;
struct S_symbol_
{
    const char * name;
    S_symbol next;
};

/* Make a unique symbol from a given string.
 *  Different calls to S_Symbol("foo") will yield the same S_symbol
 *  value, even if the "foo" strings are at different locations. */
S_symbol S_Symbol (const char *);

/* Extract the underlying string from a symbol */
const char * S_Name (S_symbol);

/* S_table is a mapping from S_symbol->any, where "any" is represented
 *     here by void*  */
typedef struct TAB_table_ * S_table;

/* Make a new table */
S_table S_Empty (void);

/* Enter a binding "sym->value" into "t", shadowing but not deleting
 *    any previous binding of "sym". */
void S_Enter (S_table t, S_symbol sym, void * value);

/* Look up the most recent binding of "sym" in "t", or return NULL
 *    if sym is unbound. */
const void * S_Look (const S_table t, const S_symbol sym);

/* Start a new "scope" in "t".  Scopes are nested. */
void S_BeginScope (S_table t);

/* Remove any bindings entered since the current scope began,
   and end the current scope. */
void S_EndScope (S_table t);

void S_Dump (S_table t, void (*show) (const S_symbol sym, const void * binding));


#endif /* end of include guard: SYMBOL_H_A9CT48M3 */
