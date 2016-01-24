%define parse.trace
%define parse.error verbose
%locations
%code requires
{
    #include <stdio.h>

    #include "util/util.h"
    #include "util/parse.h"

    #include "modules/mips/ast.h"

    #include "core/symbol.h"
    #include "core/program.h"
    #include "core/error.h"

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
        printf("MIPS Parser error %d:%d %s\n",
            yy_mips_lloc.first_line,
            yy_mips_lloc.first_column,
            message);
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
    int ival;
    const char * sval;
    A_asmStm stm;
    A_asmStmList stmList;
    A_asmOp op;
    A_asmOpList opList;
    A_asmReg reg;
    A_literal lit;
    A_var var;
}

%type <stmList> program statement_list
%type <stm>     statement
%type <opList>  operand_list
%type <op>      operand
%type <reg>     register
%type <var>     lvalue
%type <ival>    lvalue_deref
%type <lit>     literal lit_integer lit_string

%token <sval> ID
%token <sval> LIT_STR LIT_HEX LIT_DEC

%token LPAREN RPAREN LBRACK RBRACK LBRACE RBRACE
%token DOLLAR COMMA DOT COLON BACKTICK
%token NEWLINE END 0

%precedence   LOWEST

%left MINUS

%precedence   HIGHEST

%start program

%%

program:              newlines statement terminate statement_list
                      {
                          yy_mips_result = A_AsmStmList($2, $4);
                      }
                    ;
statement_list:       %empty { $$ = NULL; }
                    | statement_list statement terminate
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
operand:              lit_integer LPAREN operand RPAREN
                      {
                          $$ = A_AsmOpMem(&(@$), $1, $3);
                      }
                    | register
                      {
                          $$ = A_AsmOpReg(&(@$), $1);
                      }
                    | lvalue
                      {
                          $$ = A_AsmOpVar(&(@$), $1);
                      }
                    | literal
                      {
                          $$ = A_AsmOpLit(&(@$), $1);
                      }
                    | BACKTICK ID
                      {
                          $$ = A_AsmOpTmp(&(@$), S_Symbol($2));
                      }
                    ;
literal:              lit_integer
                    | lit_string
lit_integer:          LIT_DEC
                      {
                          Parse_status status;
                          $$ = A_LiteralInt(&(@$), Parse_DecToInt($1, &status));
                      }
                    | LIT_HEX
                      {
                          Parse_status status;
                          $$ = A_LiteralInt(&(@$), Parse_HexToInt($1, &status));
                      }
                    | MINUS lit_integer
                      {
                          //FIXME this assumes it is a signed container
                          $$->u.ival *= -1;
                      }
                    ;
lit_string:           LIT_STR
                      {
                          $$ = A_LiteralString(&(@$), $1);
                      }
                    ;
register:             DOLLAR lit_integer
                      {
                          //TODO check type
                          $$ = A_AsmRegNum(&(@$), $2->u.ival);
                      }
                    | DOLLAR ID
                      {
                          $$ = A_AsmRegName(&(@$), $2);
                      }
                    ;
lvalue:               ID
                      {
                          $$ = A_SimpleVar (&(@$), S_Symbol ($1));
                      }
                    | lvalue DOT ID
                      {
                          $$ = A_FieldVar (&(@$), $1, S_Symbol ($3), 0);
                      }
                    | lvalue lvalue_deref ID
                      {
                          $$ = A_FieldVar (&(@$), $1, S_Symbol ($3), $2);
                      }
                    ;
lvalue_deref:         COLON              { $$ = 1;      }
                    | lvalue_deref COLON { $$ = $1 + 1; }
                    ;
newlines:             %empty
                    | newlines NEWLINE
                    ;
terminate:            NEWLINE newlines
                    | END
                    ;
%%

int yydebug = 0;
