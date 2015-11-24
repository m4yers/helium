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

    #define YYLTYPE struct A_loc_

    extern Program_Module module;

    int yylex (void);

    void yyerror (const char * s);
}

%code
{
    Program_Module module = NULL;

    int Parse (Program_Module m)
    {
        module = m;
        return yyparse();
    }

    void yyerror (const char * message)
    {
        Vector_PushBack(module->errors.parser,
            Error_New(
                &yylloc,
                2000,
                "%s",
                message));
    }
}

%union
{
    int pos;
    int ival;
    string sval;
    A_var var;
    A_exp exp;
    A_expList expList;
    A_ty ty;
    A_literal literal;
    A_spec spec;
    A_specList specList;
    A_dec dec;
    A_decList decList;
    A_field field;
    A_fieldList fieldList;
    A_efield efield;
    A_efieldList efieldList;
    A_stm stm;
    A_stmList stmlist;
    A_scope scope;
}

%token <sval> ID STRING
%token <ival> INT

%token
  COMMA COLON SEMICOLON LPAREN RPAREN LBRACK RBRACK
  LBRACE RBRACE DOT
  PLUS MINUS TIMES DIVIDE EQ NEQ LT LE GT GE
  AND OR EQEQ
  IF THEN ELSE WHILE FOR TO DO IN END OF
  FN LET DEF RET
  BREAK NIL
  AUTO TYPE NEW CLASS EXTENDS METHOD PRIMITIVE IMPORT

%type <exp>
    program
        expression
            literals
            creation
            call_function
            operations
            assignment
            controls
%type <scope> scope
%type <spec> spec
%type <specList> spec_comma specs
%type <stm> stm
%type <stmlist> stm_list stm_semi
%type <var> lvalue
%type <expList> exp_list_comma exp_comma exp_list_semi exp_semi
%type <ty> type
%type <decList> declarations
%type <dec> declaration
                decl_type
                decl_variable
                decl_function
%type <efield> record_field
%type <efieldList> record_field_comma
%type <field> typed_field
%type <fieldList> typed_field_comma

%precedence   REDUCE
%precedence   TYPE

%precedence   THEN
%precedence   ELSE DO OF
%precedence   EQ
%left         OR
%left         AND
%nonassoc     GE LE EQEQ NEQ LT GT
%left         MINUS PLUS
%left         TIMES DIVIDE
%left         UMINUS
%precedence   ID
%precedence   LBRACK
%precedence   LBRACE
%precedence   LPAREN

%start program

%%

program:                  declarations { module->ast = $1; }
                        ;
stm_list:                 stm stm_semi
                        {
                            $$ = A_StmList($1, $2);
                        }
                        ;
stm_semi:                 %empty { $$ = NULL; }
                        | stm_semi stm
                        {
                            if ($1)
                            {
                                A_stmList current = $1;
                                while (current && current->tail)
                                {
                                  current = current->tail;
                                }
                                if($2)
                                {
                                    current->tail = A_StmList ($2, NULL);
                                }
                                $$ = $1;
                            }
                            else if($2)
                            {
                                $$ = A_StmList ($2, NULL);
                            }
                        }
                        ;
stm:                      expression SEMICOLON
                          {
                              $$ = A_StmExp($1);
                          }
                        | controls
                          {
                             $$ = A_StmExp($1);
                          }
                        | declaration
                          {
                              $$ = A_StmDec($1);
                          }
                        ;
expression:               literals
                        | creation
                        | lvalue %prec REDUCE { $$ = A_VarExp (0, $1); }
                        | call_function
                        | operations
                        | assignment
                        ;
literals:                 NIL     { $$ = A_NilExp (0);        }
                        | INT     { $$ = A_IntExp (0, $1);    }
                        | STRING  { $$ = A_StringExp (0, $1); }
                        ;
creation:                 LBRACK expression exp_comma RBRACK
                          {
                              $$ = A_ArrayExp (0, A_ExpList ($2, $3));
                          }
                        | LBRACE record_field record_field_comma RBRACE
                          {
                              $$ = A_RecordExp (0, NULL, A_EfieldList($2, $3));
                          }
                        | lvalue LBRACE record_field record_field_comma RBRACE
                          {
                              //TODO make it accept any lvalue
                              $$ = A_RecordExp (0, $1->u.simple, A_EfieldList($3, $4));
                          }
                        | lvalue LBRACE RBRACE
                          {
                              //TODO make it accept any lvalue
                              $$ = A_RecordExp (0, $1->u.simple, NULL);
                          }
                        ;
lvalue:                   ID
                          {
                              $$ = A_SimpleVar (0, S_Symbol ($1));
                          }
                        /* | ID LBRACK expression RBRACK */
                        /*   { */
                        /*       $$ = A_SubscriptVar (0, A_SimpleVar (0, S_Symbol ($1)), $3); */
                        /*   } */
                        | lvalue DOT ID
                          {
                              $$ = A_FieldVar (0, $1, S_Symbol ($3));
                          }
                        | lvalue LBRACK expression RBRACK
                          {
                              $$ = A_SubscriptVar (0, $1, $3);
                          }
                        ;
call_function:            lvalue LPAREN exp_list_comma RPAREN
                          {
                              // TODO make it accept any lvalue
                              $$ = A_CallExp (0, $1->u.simple, $3);
                          }
                        ;
operations:               MINUS expression %prec UMINUS
                          {
                              $$ = A_OpExp (0, A_minusOp, A_IntExp (0, 0), $2);
                          }
                        | LPAREN exp_list_semi RPAREN
                          {
                              $$ = A_SeqExp (0, $2);
                          }
                        | expression PLUS expression
                          {
                              $$ = A_OpExp (0, A_plusOp, $1, $3);
                          }
                        | expression MINUS expression
                          {
                              $$ = A_OpExp (0, A_minusOp, $1, $3);
                          }
                        | expression TIMES expression
                          {
                              $$ = A_OpExp (0, A_timesOp, $1, $3);
                          }
                        | expression DIVIDE expression
                          {
                              $$ = A_OpExp (0, A_divideOp, $1, $3);
                          }
                        | expression EQEQ expression
                          {
                              $$ = A_OpExp (0, A_eqOp, $1, $3);
                          }
                        | expression NEQ expression
                          {
                              $$ = A_OpExp (0, A_neqOp, $1, $3);
                          }
                        | expression GT expression
                          {
                              $$ = A_OpExp (0, A_gtOp, $1, $3);
                          }
                        | expression LT expression
                          {
                              $$ = A_OpExp (0, A_ltOp, $1, $3);
                          }
                        | expression GE expression
                          {
                              $$ = A_OpExp (0, A_geOp, $1, $3);
                          }
                        | expression LE expression
                          {
                              $$ = A_OpExp (0, A_leOp, $1, $3);
                          }
                        | expression AND expression
                          {
                              $$ = A_IfExp (0,
                                  $1,
                                  A_Scope(A_StmList(A_StmExp($3), NULL)),
                                  A_Scope(A_StmList(A_StmExp(A_IntExp(0, 0)), NULL)));
                          }
                        | expression OR expression
                          {
                              $$ = A_IfExp (0,
                                  $1,
                                  A_Scope(A_StmList(A_StmExp(A_IntExp(0, 1)), NULL)),
                                  A_Scope(A_StmList(A_StmExp($3), NULL)));
                          }
                        ;
assignment:               lvalue EQ expression
                          {
                              $$ = A_AssignExp (0, $1, $3);
                          }
                        ;
controls:                 IF LPAREN expression RPAREN scope
                          {
                              $$ = A_IfExp (0, $3, $5, NULL);
                          }
                        | IF LPAREN expression RPAREN scope ELSE scope
                          {
                              $$ = A_IfExp (0, $3, $5, $7);
                          }
                        | WHILE LPAREN expression RPAREN scope
                          {
                              $$ = A_WhileExp (0, $3, $5);
                          }
                        | FOR LPAREN ID EQ expression TO expression RPAREN scope
                          {
                              $$ = A_ForExp (0, S_Symbol ($3), $5, $7, $9);
                          }
                        | BREAK { $$ = A_BreakExp (0); }
                        ;
scope:                    LBRACE stm_list RBRACE
                          {
                              $$ = A_Scope($2);
                          }
                        ;
record_field:             ID EQ expression
                          {
                              $$ = A_Efield (S_Symbol ($1), $3);
                          }
                        ;
record_field_comma:       %empty { $$ = NULL; }
                        | record_field_comma COMMA record_field
                          {
                              if ($1)
                              {
                                  A_efieldList current = $1;
                                  while (current && current->tail)
                                  {
                                    current = current->tail;
                                  }
                                  current->tail = A_EfieldList ($3, NULL);
                                  $$ = $1;
                              }
                              else
                              {
                                  $$ = A_EfieldList ($3, NULL);
                              }
                          }
                        ;
exp_list_comma:           %empty { $$ = NULL; }
                        | expression exp_comma
                          {
                              $$ = A_ExpList ($1, $2);
                          }
                        ;
exp_comma:                %empty { $$ = NULL; }
                        | exp_comma COMMA expression
                          {
                              if ($1)
                              {
                                  A_expList current = $1;
                                  while (current && current->tail)
                                  {
                                    current = current->tail;
                                  }
                                  current->tail = A_ExpList ($3, NULL);
                                  $$ = $1;
                              }
                              else
                              {
                                  $$ = A_ExpList ($3, NULL);
                              }
                          }
                        ;
exp_list_semi:            %empty { $$ = NULL; }
                        | expression exp_semi
                          {
                              $$ = A_ExpList ($1, $2);
                          }
                        ;
exp_semi:                 %empty { $$ = NULL; }
                        | exp_semi SEMICOLON expression
                          {
                              if ($1)
                              {
                                  A_expList current = $1;
                                  while (current && current->tail)
                                  {
                                    current = current->tail;
                                  }
                                  current->tail = A_ExpList ($3, NULL);
                                  $$ = $1;
                              }
                              else
                              {
                                  $$ = A_ExpList ($3, NULL);
                              }
                          }
                        ;
spec:                     type
                          {
                              $$ = A_SpecType(0, $1);
                          }
                        | INT
                          {
                              $$ = A_SpecLiteral(0, A_LiteralInt(0, $1));
                          }
                        | STRING
                          {
                              $$ = A_SpecLiteral(0, A_LiteralString(0, $1));
                          }
                        ;
spec_comma:                %empty { $$ = NULL; }
                        | spec_comma COMMA spec
                          {
                              if ($1)
                              {
                                  A_specList current = $1;
                                  while (current && current->tail)
                                  {
                                    current = current->tail;
                                  }
                                  current->tail = A_SpecList ($3, NULL);
                                  $$ = $1;
                              }
                              else
                              {
                                  $$ = A_SpecList ($3, NULL);
                              }
                          }
                        ;
specs:                    LT spec spec_comma GT
                          {
                              $$ = A_SpecList($2, $3);
                          }
                        ;
declarations:             %empty { $$ = NULL; }
                        | declarations declaration
                          {
                              if ($1)
                              {
                                  A_decList current = $1;
                                  while (current && current->tail)
                                  {
                                    current = current->tail;
                                  }
                                  current->tail = A_DecList ($2, NULL);
                                  $$ = $1;
                              }
                              else
                              {
                                  $$ = A_DecList ($2, NULL);
                              }
                          }
                        ;
declaration:              decl_type
                        | decl_variable
                        | decl_function
                        ;
decl_type:                DEF ID EQ type
                          {
                              $$ = A_TypeDec (S_Symbol($2), $4);
                          }
                        ;
decl_variable:            LET ID COLON type SEMICOLON
                          {
                              $$ = A_VarDec (0, S_Symbol ($2), $4, NULL);
                          }
                        | LET ID COLON type EQ expression SEMICOLON
                          {
                              $$ = A_VarDec (0, S_Symbol ($2), $4, $6);
                          }
                        | LET ID EQ expression SEMICOLON
                          {
                              $$ = A_VarDec (0, S_Symbol ($2), NULL, $4);
                          }
                        ;
decl_function:            FN ID scope
                          {
                              $$ = A_FunctionDec (
                                  0,
                                  S_Symbol ($2),
                                  NULL,
                                  NULL,
                                  $3);
                          }
                        | FN ID LPAREN RPAREN scope
                          {
                              $$ = A_FunctionDec (
                                  0,
                                  S_Symbol ($2),
                                  NULL,
                                  NULL,
                                  $5);
                          }
                        | FN ID LPAREN typed_field typed_field_comma RPAREN scope
                          {
                              $$ = A_FunctionDec (
                                  0,
                                  S_Symbol ($2),
                                  A_FieldList ($4, $5),
                                  NULL,
                                  $7);
                          }
                        | FN ID COLON type scope
                          {
                              $$ = A_FunctionDec (
                                  0,
                                  S_Symbol ($2),
                                  NULL,
                                  $4,
                                  $5);
                          }
                        | FN ID LPAREN RPAREN COLON type scope
                          {
                              $$ = A_FunctionDec (
                                  0,
                                  S_Symbol ($2),
                                  NULL,
                                  $6,
                                  $7);
                          }
                        | FN ID LPAREN typed_field typed_field_comma RPAREN COLON type scope
                          {
                              $$ = A_FunctionDec (
                                  0,
                                  S_Symbol ($2),
                                  A_FieldList ($4, $5),
                                  $8,
                                  $9);
                          }
                        ;
type:                     LBRACE typed_field typed_field_comma RBRACE
                          {
                              $$ = A_RecordTy (0, A_FieldList ($2, $3));
                          }
                        | LBRACK expression exp_semi RBRACK
                          {
                              $$ = A_ArrayTy (0, A_ExpList($2, $3));
                          }
                        | ID specs
                          {
                              $$ = A_NameTy (0, S_Symbol($1), $2);
                          }
                        | ID
                          {
                              $$ = A_NameTy (0, S_Symbol($1), NULL);
                          }
                        ;
typed_field_comma:         %empty { $$ = NULL; }
                        | typed_field_comma COMMA typed_field
                          {
                              if ($1)
                              {
                                  A_fieldList current = $1;
                                  while (current && current->tail)
                                  {
                                    current = current->tail;
                                  }
                                  current->tail = A_FieldList ($3, NULL);
                                  $$ = $1;
                              }
                              else
                              {
                                  $$ = A_FieldList ($3, NULL);
                              }
                          }
                        ;
typed_field:              ID COLON type
                          {
                              // id: type
                              $$ = A_Field (&yylloc, S_Symbol($1), $3);
                          }
                        ;

%%

int yydebug = 0;
