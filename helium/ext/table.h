#ifndef TABLE_H_WONFRLUQ
#define TABLE_H_WONFRLUQ

typedef struct TAB_table_ * TAB_table;

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

#endif /* end of include guard: TABLE_H_WONFRLUQ */
