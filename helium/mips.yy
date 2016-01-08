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

    extern int yy_mips_column;
    extern int yy_mips_line;

    int yy_mips_lex (void);

    void yy_mips_error (const char * s);

    int MIPSParse (A_loc loc);
}

%code
{
    //TODO make it accept different type of parse trees
    // Program_Module module = NULL;

    int MIPSParse (A_loc loc)
    {
        yy_mips_lloc = *loc;
        yy_mips_column = loc->first_column;
        yy_mips_line = loc->first_line;
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
    A_asmReg reg;
}

%type <stmList> program statement_list
%type <stm>     statement
%type <opList>  operand_list
%type <op>      operand
%type <reg>     register

%token <sval> ID STRING
%token <ival> INT

%token DOLLAR COMMA NEWLINE LPAREN RPAREN MINUS COLON

%precedence   LOWEST

%precedence   HIGHEST

%start program

%%

program:              newlines statement statement_list newlines
                      {
                          yy_mips_result = A_AsmStmList($2, $3);
                      }
                    ;
statement_list:       %empty { $$ = NULL; }
                    | statement_list newlines statement
                      {
                          if ($1)
                          {
                              A_asmStmList current = $1;
                              while (current && current->tail)
                              {
                                current = current->tail;
                              }
                              current->tail = A_AsmStmList ($3, NULL);
                              $$ = $1;
                          }
                          else
                          {
                              $$ = A_AsmStmList ($3, NULL);
                          }
                      }
                    ;
statement:            ID
                      {
                          $$ = A_AsmStmInst(&(@$), $1, NULL);
                      }
                    | ID operand operand_list
                      {
                          $$ = A_AsmStmInst(&(@$), $1, A_AsmOpList($2, $3));
                      }
                    | ID COLON
                      {
                          $$ = A_AsmStmLab(&(@$), $1);
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
operand:              INT LPAREN register RPAREN
                      {
                          $$ = A_AsmOpMem(&(@$), $1, $3);
                      }
                    | register
                      {
                          $$ = A_AsmOpReg(&(@$), $1);
                      }
                    | MINUS INT
                      {
                          $$ = A_AsmOpInt(&(@$), -$2);
                      }
                    | INT
                      {
                          $$ = A_AsmOpInt(&(@$), $1);
                      }
                    ;
register:             DOLLAR INT
                      {
                          $$ = A_AsmRegNum(&(@$), $2);
                      }
                    | DOLLAR ID
                      {
                          $$ = A_AsmRegName(&(@$), $2);
                      }
                    ;
newlines:             %empty
                    | newlines NEWLINE
%%

int yydebug = 0;
