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

    #define YY_HELIUM_LTYPE struct A_loc_t

    extern Program_Module module;
    extern A_asmStmList ParseAsm(const char * input);

    int yy_helium_lex (void);

    void yy_helium_error (const char * s);
}

%code
{
    Program_Module module = NULL;

    int HeliumParse (Program_Module m)
    {
        module = m;
        return yy_helium_parse();
    }

    void yy_helium_error (const char * message)
    {
    printf("string: '%s'\n", yy_helium_lval.sval);
        Vector_PushBack(&module->errors.parser,
            Error_New(
                &yy_helium_lloc,
                2000,
                "%s",
                message));
    }
}

%union
{
    int pos;
    int ival;
    char * sval;
    A_var var;
    A_exp exp;
    A_expList expList;
    A_ty ty;
    A_literal literal;
    /* A_spec spec; */
    /* A_specList specList; */
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
  EMARK PLUS MINUS STAR DIVIDE EQ NEQ LT LE GT GE
  AND OR EQEQ AS
  IF THEN ELSE WHILE FOR TO DO IN OF
  FN MACRO LET DEF RET ASM
  BREAK AMP NIL
  AUTO TYPE NEW CLASS EXTENDS METHOD PRIMITIVE IMPORT

%type <exp>
    program
        expression
            literals
            creation
            call_function
            call_macro
            operations
            assignment
            controls
            asm
%type <scope> scope
/* %type <spec> spec */
/* %type <specList> spec_comma specs */
%type <stm> stm
%type <stmlist> stm_list stm_semi
%type <var> lvalue
%type <expList> exp_list_comma exp_comma
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
%type <ival> lvalue_jumps

%precedence   LOWEST

%precedence   EMARK
%precedence   ASM

%precedence   AS TYPE

%precedence   THEN
%precedence   ELSE DO OF
%precedence   EQ
%left         OR
%left         AND
%nonassoc     GE LE EQEQ NEQ LT GT
%left         MINUS PLUS
%left         STAR DIVIDE
%left         UMINUS
%left         DOT COLON
%precedence   ID
%precedence   LBRACK
%precedence   LBRACE
%precedence   LPAREN RPAREN

%precedence   HIGHEST

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
type:                     AMP type
                          {
                              $$ = A_PointerTy(&(@$), $2);
                          }
                        | LBRACE typed_field typed_field_comma RBRACE
                          {
                              $$ = A_RecordTy (&(@$), A_FieldList ($2, $3));
                          }
                        | LBRACK type SEMICOLON expression RBRACK
                          {
                              $$ = A_ArrayTy (&(@$), $2, $4);
                          }
                        /* | ID specs */
                        /*   { */
                        /*       $$ = A_NameTy (&(@$), S_Symbol($1), $2); */
                        /*   } */
                        | ID
                          {
                              $$ = A_NameTy (&(@$), S_Symbol($1), NULL);
                          }
                        ;
typed_field_comma:        %empty { $$ = NULL; }
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
                              $$ = A_Field (&(@$), S_Symbol($1), $3);
                          }
                        ;
expression:               literals
                        | creation
                        | lvalue %prec LOWEST { $$ = A_VarExp (&(@$), $1); }
                        | call_function
                        | call_macro
                        | operations
                        | assignment
                        | asm
                        | AMP lvalue
                          {
                              $$ = A_AddressOfExp(&(@$), $2);
                          }
                        | STAR expression
                          {
                              $$ = A_ValueAtExp(&(@$), $2);
                          }
                        | expression AS type
                          {
                              $$ = A_TypeCastExp(&(@$), $3, $1);
                          }
                        ;
asm:                      ASM LBRACE STRING RBRACE
                          {
                              $$ = A_AsmExp(&(@$), ParseAsm($3), NULL, NULL);
                          }
                        | ASM LPAREN
                                  SEMICOLON exp_list_comma
                                  SEMICOLON exp_list_comma
                              RPAREN
                            LBRACE STRING RBRACE
                          {
                              $$ = A_AsmExp(&(@$), ParseAsm($9), $4, $6);
                          }
                        ;
literals:                 NIL     { $$ = A_NilExp (&(@$));        }
                        | INT     { $$ = A_IntExp (&(@$), $1);    }
                        | STRING  { $$ = A_StringExp (&(@$), $1); }
                        ;
creation:                 LBRACK expression exp_comma RBRACK
                          {
                              $$ = A_ArrayExp (&(@$), A_ExpList ($2, $3));
                          }
                        | LBRACE record_field record_field_comma RBRACE
                          {
                              $$ = A_RecordExp (&(@$), NULL, A_EfieldList($2, $3));
                          }
                        | lvalue LBRACE record_field record_field_comma RBRACE
                          {
                              //TODO make it accept any lvalue
                              $$ = A_RecordExp (&(@$), $1->u.simple, A_EfieldList($3, $4));
                          }
                        | lvalue LBRACE RBRACE
                          {
                              //TODO make it accept any lvalue
                              $$ = A_RecordExp (&(@$), $1->u.simple, NULL);
                          }
                        ;
lvalue:                   ID
                          {
                              $$ = A_SimpleVar (&(@$), S_Symbol ($1));
                          }
                        | lvalue DOT ID
                          {
                              $$ = A_FieldVar (&(@$), $1, S_Symbol ($3), 0);
                          }
                        | lvalue lvalue_jumps ID
                          {
                              $$ = A_FieldVar (&(@$), $1, S_Symbol ($3), $2);
                          }
                        | lvalue LBRACK expression RBRACK
                          {
                              $$ = A_SubscriptVar (&(@$), $1, $3);
                          }
                        ;
lvalue_jumps:             COLON              { $$ = 1;      }
                        | lvalue_jumps COLON { $$ = $1 + 1; }
                        ;
call_function:            lvalue LPAREN exp_list_comma RPAREN
                          {
                              // TODO make it accept any lvalue
                              $$ = A_CallExp (&(@$), $1->u.simple, $3);
                          }
                        ;
call_macro:               lvalue EMARK LPAREN exp_list_comma RPAREN
                          {
                              // TODO make it accept any lvalue
                              $$ = A_MacroCallExp (&(@$), $1->u.simple, $4);
                          }
                        ;
operations:               MINUS expression %prec UMINUS
                          {
                              $$ = A_OpExp (&(@$), A_minusOp, A_IntExp (&(@$), 0), $2);
                          }
                        | LPAREN expression RPAREN
                          {
                              $$ = $2;
                          }
                        | expression PLUS expression
                          {
                              $$ = A_OpExp (&(@$), A_plusOp, $1, $3);
                          }
                        | expression MINUS expression
                          {
                              $$ = A_OpExp (&(@$), A_minusOp, $1, $3);
                          }
                        | expression STAR expression
                          {
                              $$ = A_OpExp (&(@$), A_timesOp, $1, $3);
                          }
                        | expression DIVIDE expression
                          {
                              $$ = A_OpExp (&(@$), A_divideOp, $1, $3);
                          }
                        | expression EQEQ expression
                          {
                              $$ = A_OpExp (&(@$), A_eqOp, $1, $3);
                          }
                        | expression NEQ expression
                          {
                              $$ = A_OpExp (&(@$), A_neqOp, $1, $3);
                          }
                        | expression GT expression
                          {
                              $$ = A_OpExp (&(@$), A_gtOp, $1, $3);
                          }
                        | expression LT expression
                          {
                              $$ = A_OpExp (&(@$), A_ltOp, $1, $3);
                          }
                        | expression GE expression
                          {
                              $$ = A_OpExp (&(@$), A_geOp, $1, $3);
                          }
                        | expression LE expression
                          {
                              $$ = A_OpExp (&(@$), A_leOp, $1, $3);
                          }
                        | expression AND expression
                          {
                              $$ = A_IfExp (&(@$),
                                  $1,
                                  A_Scope(A_StmList(A_StmExp($3), NULL)),
                                  A_Scope(A_StmList(A_StmExp(A_IntExp(&(@$), 0)), NULL)));
                          }
                        | expression OR expression
                          {
                              $$ = A_IfExp (&(@$),
                                  $1,
                                  A_Scope(A_StmList(A_StmExp(A_IntExp(&(@$), 1)), NULL)),
                                  A_Scope(A_StmList(A_StmExp($3), NULL)));
                          }
                        ;
assignment:               lvalue EQ expression
                          {
                              $$ = A_AssignExp (&(@$), $1, $3);
                          }
                        ;
controls:                 IF LPAREN expression RPAREN scope
                          {
                              $$ = A_IfExp (&(@$), $3, $5, NULL);
                          }
                        | IF LPAREN expression RPAREN scope ELSE scope
                          {
                              $$ = A_IfExp (&(@$), $3, $5, $7);
                          }
                        | WHILE LPAREN expression RPAREN scope
                          {
                              $$ = A_WhileExp (&(@$), $3, $5);
                          }
                        | FOR LPAREN ID EQ expression TO expression RPAREN scope
                          {
                              $$ = A_ForExp (&(@$), S_Symbol ($3), $5, $7, $9);
                          }
                        | BREAK { $$ = A_BreakExp (&(@$)); }
                        | RET expression SEMICOLON %prec LOWEST { $$ = A_RetExp (&(@$), $2); }
                        ;
scope:                    LBRACE stm_list RBRACE
                          {
                              $$ = A_Scope($2);
                          }
                        | LBRACE RBRACE
                          {
                              $$ = A_Scope(NULL);
                          }
                        ;
record_field:             ID EQ expression
                          {
                              $$ = A_Efield (&(@$), S_Symbol ($1), $3);
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
/* spec:                     type */
/*                           { */
/*                               $$ = A_SpecType(&(@$), $1); */
/*                           } */
/*                         | INT */
/*                           { */
/*                               $$ = A_SpecLiteral(&(@$), A_LiteralInt(&(@$), $1)); */
/*                           } */
/*                         | STRING */
/*                           { */
/*                               $$ = A_SpecLiteral(&(@$), A_LiteralString(&(@$), $1)); */
/*                           } */
/*                         ; */
/* spec_comma:                %empty { $$ = NULL; } */
/*                         | spec_comma COMMA spec */
/*                           { */
/*                               if ($1) */
/*                               { */
/*                                   A_specList current = $1; */
/*                                   while (current && current->tail) */
/*                                   { */
/*                                     current = current->tail; */
/*                                   } */
/*                                   current->tail = A_SpecList ($3, NULL); */
/*                                   $$ = $1; */
/*                               } */
/*                               else */
/*                               { */
/*                                   $$ = A_SpecList ($3, NULL); */
/*                               } */
/*                           } */
/*                         ; */
/* specs:                    LT spec spec_comma GT */
/*                           { */
/*                               $$ = A_SpecList($2, $3); */
/*                           } */
/*                         ; */
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
                              $$ = A_TypeDec (&(@$), S_Symbol($2), $4);
                          }
                        ;
decl_variable:            LET ID COLON type SEMICOLON
                          {
                              $$ = A_VarDec (&(@$), S_Symbol ($2), $4, NULL);
                          }
                        | LET ID COLON type EQ expression SEMICOLON
                          {
                              $$ = A_VarDec (&(@$), S_Symbol ($2), $4, $6);
                          }
                        | LET ID EQ expression SEMICOLON
                          {
                              $$ = A_VarDec (&(@$), S_Symbol ($2), NULL, $4);
                          }
                        ;
decl_function:            FN ID scope
                          {
                              $$ = A_FunctionDec (
                                  &(@$),
                                  S_Symbol ($2),
                                  NULL,
                                  NULL,
                                  $3);
                          }
                        | FN ID LPAREN RPAREN scope
                          {
                              $$ = A_FunctionDec (
                                  &(@$),
                                  S_Symbol ($2),
                                  NULL,
                                  NULL,
                                  $5);
                          }
                        | FN ID LPAREN typed_field typed_field_comma RPAREN scope
                          {
                              $$ = A_FunctionDec (
                                  &(@$),
                                  S_Symbol ($2),
                                  A_FieldList ($4, $5),
                                  NULL,
                                  $7);
                          }
                        | FN ID COLON type scope
                          {
                              $$ = A_FunctionDec (
                                  &(@$),
                                  S_Symbol ($2),
                                  NULL,
                                  $4,
                                  $5);
                          }
                        | FN ID LPAREN RPAREN COLON type scope
                          {
                              $$ = A_FunctionDec (
                                  &(@$),
                                  S_Symbol ($2),
                                  NULL,
                                  $6,
                                  $7);
                          }
                        | FN ID LPAREN typed_field typed_field_comma RPAREN COLON type scope
                          {
                              $$ = A_FunctionDec (
                                  &(@$),
                                  S_Symbol ($2),
                                  A_FieldList ($4, $5),
                                  $8,
                                  $9);
                          }
                        ;

%%

int yydebug = 0;
