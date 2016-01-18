#ifndef TABLE_H_WONFRLUQ
#define TABLE_H_WONFRLUQ

// SHIT this must be rewritten into something more suitable

#define TABSIZE 127

typedef struct record_t
{
    const void * key;
    const void * value;
    /*
     * next item in the current slot
     */
    struct record_t * next;

    /*
     * thread that leads to the previously added record
     */
    struct record_t * prev_added;
} * record;

typedef struct TAB_table_t
{
    record table[TABSIZE];
    const void * top;
} * TAB_table;

/* Make a new table mapping "keys" to "values". */
TAB_table TAB_Empty (void);

/* Enter the mapping "key"->"value" into table "t",
 *    shadowing but not destroying any previous binding for "key". */
void TAB_Enter (TAB_table t, const void * key, const void * value);

/* Look up the most recent binding for "key" in table "t" */
const void * TAB_Look (TAB_table t, const void * key);

/* Pop the most recent binding and return its key.
 * This may expose another binding for the same key, if there was one. */
void * TAB_Pop (TAB_table t);

/** Returns keys of the table */
void ** TAB_Keys (TAB_table t);

/* Call "show" on every "key"->"value" pair in the table,
 *  including shadowed bindings, in order from the most
 *  recent binding of any key to the oldest binding in the table */
void TAB_Dump (TAB_table t, void (*show) (const void * key, const void * value));

/*
 * This loop traverses the threaded hash table starting from the last added key~value pair till
 * it reaches the very first added pair.
 *
 * DARK MAGIC: since C(or rather certain compilers) does not allow to declare anonymous structs
 * inside for loop i had to add one additional loop to obtain the very top record in the table,
 * after this only the second loop is used to iterate over table records.
 */
#define TAB_FOREACH(k, v, t)\
    for (record __r = t->top ? t->table[((unsigned)t->top) % TABSIZE] : NULL; __r; __r = NULL)   \
        for (const void                                                                          \
                * k = __r->key,                                                                  \
                * v = __r->value;                                                                \
                k != NULL;                                                                       \
                __r = __r->prev_added,                                                           \
                k = __r ? __r->key : NULL,                                                       \
                v = __r ? __r->value : NULL)

#endif /* end of include guard: TABLE_H_WONFRLUQ */
