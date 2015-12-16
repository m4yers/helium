#ifndef TABLE_H_WONFRLUQ
#define TABLE_H_WONFRLUQ

#define TABSIZE 127

typedef struct binder_t
{
    const void * key;
    const void * value;
    struct binder_t * next;
    const void * prevtop;
} * binder;

typedef struct TAB_table_t
{
    binder table[TABSIZE];
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
 * inside for loop i had to add two additional loops to scope additional variables. First loop
 * runs only once declaring index iterator, next loop stores table binder(record), the third one
 * splits record into key and value pair.
 */
#define TAB_FOREACH(k, v, t)                                                            \
    for (int __i = ((unsigned)t->top) % TABSIZE; __i != 0; __i = 0)                     \
        for (binder __b = t->table[__i]; __b != NULL; __b = NULL)                       \
            for (                                                                       \
                const void * k = __b->key, * v = __b->value;                            \
                k != NULL;                                                              \
                __i = ((unsigned)__b->prevtop) % TABSIZE,                               \
                __b = __i != 0 ? t->table[__i] : NULL,                                  \
                k = NULL,                                                               \
                v = NULL)

#endif /* end of include guard: TABLE_H_WONFRLUQ */
