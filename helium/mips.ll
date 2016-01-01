%{
    #include <string.h>

    #include "ext/vector.h"
    #include "ext/mem.h"

    #include "mips.yy.h"

    #define YY_USER_ACTION adjust();

    int yy_mips_column = 1;

    static void adjust (void)
    {
        yy_mips_lloc.token = strdup(yytext);
        yy_mips_lloc.first_line = yy_mips_lloc.last_line = yylineno;
        yy_mips_lloc.first_column = yy_mips_column;
        yy_mips_lloc.last_column = yy_mips_column + yyleng - 1;
        yy_mips_column += yyleng;
    }
%}

%option noyywrap
%option yylineno

DIGIT    [0-9]
ID       [_a-zA-Z][_a-zA-Z0-9]*

%%

" "|\t           { continue; }
\n               { yy_mips_column = 1; return NEWLINE; }
"$"              { return DOLLAR; }
","              { return COMMA; }
"("              { return LPAREN; }
")"              { return RPAREN; }
{DIGIT}+         { yy_mips_lval.ival = atoi(yy_mips_text);    return INT; }
{ID}             { yy_mips_lval.sval = strdup (yy_mips_text); return ID;  }
.                {
                    // Vector_PushBack(&module->errors.lexer,
                    //     Error_New(
                    //         &yy_mips_lloc,
                    //         1100,
                    //         "Unknown token %s",
                    //         yy_mips_lloc.token));
                 }
