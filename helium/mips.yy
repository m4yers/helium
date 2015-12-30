%define parse.trace
%define parse.error verbose
%locations
%code requires
{
    #include <stdio.h>

    #include "ext/util.h"

    #include "symbol.h"
    #include "program.h"
    #include "error.h"
    #include "ast.h"
    #include "ast_asm.h"

    #define YY_MIPS_LTYPE struct A_loc_t

    extern A_asmStmList yy_mips_result;

    int yy_mips_lex (void);

    void yy_mips_error (const char * s);
}

%code
{
    //TODO make it accept different type of parse trees
    // Program_Module module = NULL;

    int MIPSParse ()
    {
        return yy_mips_parse();
    }

    void yy_mips_error (const char * message)
    {
        printf("MIPS Parser error %s\n", message);
        // Vector_PushBack(&module->errors.parser,
        //     Error_New(
        //         &yy_mips_lloc,
        //         2100,
        //         "%s",
        //         message));
    }
}

%union
{
    int pos;
    int ival;
    char * sval;
    A_asmStm stm;
    A_asmStmList stmList;
    A_asmOp op;
    A_asmOpList opList;
}

%type <stmList> program statement_list
%type <stm>     statement
%type <opList>  operand_list
%type <op>      operand

%token <sval> ID STRING
%token <ival> INT

%token DOLLAR COMMA NEWLINE

%precedence   LOWEST

%precedence   HIGHEST

%start program

%%

program:              statement statement_list { yy_mips_result = A_AsmStmList($1, $2); }
                    ;
statement_list:       %empty { $$ = NULL; }
                    | statement_list statement
                      {
                          if ($1)
                          {
                              A_asmStmList current = $1;
                              while (current && current->tail)
                              {
                                current = current->tail;
                              }
                              current->tail = A_AsmStmList ($2, NULL);
                              $$ = $1;
                          }
                          else
                          {
                              $$ = A_AsmStmList ($2, NULL);
                          }
                      }
                    ;
statement:            ID operand operand_list
                      {
                          $$ = A_AsmStmInst(&(@$), $1, A_AsmOpList($2, $3));
                      }
                    ;
operand_list:         %empty { $$ = NULL; }
                    | operand_list COMMA operand
                      {
                          if ($1)
                          {
                              A_asmOpList current = $1;
                              while (current && current->tail)
                              {
                                current = current->tail;
                              }
                              current->tail = A_AsmOpList ($3, NULL);
                              $$ = $1;
                          }
                          else if($3)
                          {
                              $$ = A_AsmOpList ($3, NULL);
                          }
                      }
                    ;
operand:              DOLLAR INT
                      {
                          $$ = A_AsmOpRegNum(&(@$), $2);
                      }
                    | DOLLAR ID
                      {
                          $$ = A_AsmOpRegName(&(@$), $2);
                      }
                    | INT
                      {
                          $$ = A_AsmOpImm(&(@$), $1);
                      }
%%

int yydebug = 0;
