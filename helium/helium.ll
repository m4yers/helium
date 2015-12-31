%{
    #include <string.h>

    #include "ext/vector.h"
    #include "ext/mem.h"

    #include "helium.yy.h"

    #define YY_USER_ACTION adjust();

    int yy_helium_column = 1;

    static void adjust (void)
    {
        yy_helium_lloc.token = strdup(yytext);
        yy_helium_lloc.first_line = yy_helium_lloc.last_line = yylineno;
        yy_helium_lloc.first_column = yy_helium_column;
        yy_helium_lloc.last_column = yy_helium_column + yyleng - 1;
        yy_helium_column += yyleng;
    }
%}

%option noyywrap
%option yylineno

%s STATE_ASM_OPTS
%s STATE_ASM
%x STATE_BLOCK_COMMENT
%x STATE_LINE_COMMENT
%x STATE_STRING

DIGIT    [0-9]
ID       [_a-zA-Z][_a-zA-Z0-9]*

%%

"/*"             BEGIN(STATE_BLOCK_COMMENT);
<STATE_BLOCK_COMMENT>
{
                 /** HMM does this preserve \n in the sval?*/
    \n           { yy_helium_column = 1; continue;                                      }
    [^*]*        /** eat anything that's not a '*' */
    "*"+[^*/]*   /** eat up '*'s not followed by '/'s */
    "*"+"/"      BEGIN(INITIAL);
}

"//"             BEGIN(STATE_LINE_COMMENT);
<STATE_LINE_COMMENT>
{
    .*           /** eat up all characters */
    "\n"         { yy_helium_column = 1; BEGIN(INITIAL);                                }
}

\"               BEGIN(STATE_STRING);
<STATE_STRING>
{
    [^"]*        { yy_helium_lval.sval = strdup (yy_helium_text);                       }
    \"           { BEGIN(INITIAL); return STRING;                                       }
}

asm              { BEGIN(STATE_ASM_OPTS); return ASM;                                   }
<STATE_ASM_OPTS>
{
    ")"      { BEGIN(STATE_ASM); return RPAREN;                                         }
    "{"      { BEGIN(STATE_ASM); return LBRACE;                                         }
}
<STATE_ASM>
{
    [" "|\t|\n]* { continue;                                                            }
    [^\{\}]*     { yy_helium_lval.sval = strdup (yy_helium_text); return STRING;        }
    "}"          { BEGIN(INITIAL); return RBRACE;                                       }
}

" "|\t           { continue; }
\n               { yy_helium_column = 1; continue; }
"!"              { return EMARK; }
","              { return COMMA; }
":"              { return COLON; }
";"              { return SEMICOLON; }
"("              { return LPAREN; }
")"              { return RPAREN; }
"["              { return LBRACK; }
"]"              { return RBRACK; }
"{"              { return LBRACE; }
"}"              { return RBRACE; }
"."              { return DOT; }
"+"              { return PLUS; }
"-"              { return MINUS; }
"*"              { return STAR; }
"/"              { return DIVIDE; }
"="              { return EQ; }
"=="             { return EQEQ; }
"<>"             { return NEQ; }
"<"              { return LT; }
"<="             { return LE; }
">"              { return GT; }
">="             { return GE; }
"&&"             { return AND; }
"||"             { return OR; }
"&"              { return AMP; }
{DIGIT}+         { yy_helium_lval.ival=atoi(yy_helium_text); return INT; }
fn               { return FN; }
macro            { return MACRO; }
ret              { return RET; }
let              { return LET; }
def              { return DEF; }
as               { return AS; }
if               { return IF; }
then             { return THEN; }
else             { return ELSE; }
while            { return WHILE; }
for              { return FOR; }
to               { return TO; }
do               { return DO; }
in               { return IN; }
of               { return OF; }
break            { return BREAK; }
nil              { return NIL; }
auto             { return AUTO; }
type             { return TYPE; }
new              { return NEW; }
class            { return CLASS; }
extends          { return EXTENDS; }
method           { return METHOD; }
volatile         { return VOLATILE; }
promitive        { return PRIMITIVE; }
import           { return IMPORT; }
{ID}             { yy_helium_lval.sval = strdup (yy_helium_text); return ID;}
.                {
                    Vector_PushBack(&module->errors.lexer,
                        Error_New(
                            &yy_helium_lloc,
                            1000,
                            "Unknown token %s",
                            yy_helium_lloc.token));
                 }
