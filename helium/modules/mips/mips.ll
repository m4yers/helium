%{
    #include <string.h>

    #include "util/vector.h"
    #include "util/mem.h"

    #include "mips.yy.h"

    #define YY_USER_ACTION lloc_adjust();

    int yy_mips_column = 1;
    int yy_mips_line = 1;

    static void lloc_adjust (void)
    {
        yy_mips_lloc.token = strdup(yytext);
        yy_mips_lloc.first_line = yy_mips_lloc.last_line = yy_mips_line;
        yy_mips_lloc.first_column = yy_mips_column;
        yy_mips_lloc.last_column = yy_mips_column + yyleng - 1;
        yy_mips_column += yyleng;
    }

    static void lloc_newline(void)
    {
        yy_mips_column = 1;
        yy_mips_line++;
    }
%}

%option noyywrap

%x STATE_LINE_COMMENT

DIGIT    [0-9]
ID       [_a-zA-Z][_a-zA-Z0-9]*

%%

"//"             { BEGIN(STATE_LINE_COMMENT);                                           }
<STATE_LINE_COMMENT>
{
    \n           { lloc_newline(); BEGIN(INITIAL); return NEWLINE;                      }
    .*           /** eat up all characters */
}

" "|\t           { continue;                                                            }
\n               { lloc_newline(); return NEWLINE;                                      }
"$"              { return DOLLAR;                                                       }
","              { return COMMA;                                                        }
":"              { return COLON;                                                        }
"."              { return DOT;                                                          }
"("              { return LPAREN;                                                       }
")"              { return RPAREN;                                                       }
"["              { return LBRACK;                                                       }
"]"              { return RBRACK;                                                       }
"{"              { return LBRACE;                                                       }
"}"              { return RBRACE;                                                       }
"-"              { return MINUS;                                                        }
{DIGIT}+         { yy_mips_lval.ival = atoi(yy_mips_text);    return INT;               }
{ID}             { yy_mips_lval.sval = strdup (yy_mips_text); return ID;                }
.                {
                    // Vector_PushBack(&module->errors.lexer,
                    //     Error_New(
                    //         &yy_mips_lloc,
                    //         1100,
                    //         "Unknown token %s",
                    //         yy_mips_lloc.token));
                 }
