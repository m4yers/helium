#ifndef AST_H_50UWFKJ9
#define AST_H_50UWFKJ9

/**************
 *  Location  *
 **************/

// TODO move to top level ast header
typedef struct A_loc_t
{
    int first_line;
    int first_column;
    int last_line;
    int last_column;
    //NOTE token MUST NOT be used outside lexer and parser
    const char * token;
} * A_loc;

#endif /* end of include guard: AST_H_50UWFKJ9 */
