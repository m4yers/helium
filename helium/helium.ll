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

%x block_comment
%x line_comment
%x string

DIGIT    [0-9]
ID       [_a-zA-Z][_a-zA-Z0-9]*

%%

"/*"             BEGIN(block_comment);
<block_comment>
{
                 /** HMM does this preserve \n in the sval?*/
    \n           { yy_helium_column = 1; continue; }
    [^*]*        /** eat anything that's not a '*' */
    "*"+[^*/]*   /** eat up '*'s not followed by '/'s */
    "*"+"/"      BEGIN(INITIAL);
}

"//"             BEGIN(line_comment);
<line_comment>
{
    .*           /** eat up all characters */
    "\n"         { yy_helium_column = 1; BEGIN(INITIAL); }
}

\"               BEGIN(string);
<string>
{
    [^"]*        { yy_helium_lval.sval = strdup (yy_helium_text); }
    \"           { BEGIN(INITIAL); return STRING; }
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
asm              { return ASM; }
of               { return OF; }
break            { return BREAK; }
nil              { return NIL; }
auto             { return AUTO; }
type             { return TYPE; }
new              { return NEW; }
class            { return CLASS; }
extends          { return EXTENDS; }
method           { return METHOD; }
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
