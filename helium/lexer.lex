%{
    #include <string.h>

    #include "ext/vector.h"
    #include "ext/mem.h"

    #include "parser.h"

    #define YY_USER_ACTION adjust();

    int yycolumn = 1;

    static void adjust (void)
    {
        yylloc.token = strdup(yytext);
        yylloc.first_line = yylloc.last_line = yylineno;
        yylloc.first_column = yycolumn;
        yylloc.last_column = yycolumn + yyleng - 1;
        yycolumn += yyleng;
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
    \n           { yycolumn = 1; continue; }
    [^*]*        /** eat anything that's not a '*' */
    "*"+[^*/]*   /** eat up '*'s not followed by '/'s */
    "*"+"/"      BEGIN(INITIAL);
}

"//"             BEGIN(line_comment);
<line_comment>
{
    "\n"         { yycolumn = 1; BEGIN(INITIAL); }
    ".*"           /** eat anything */
}

\"               BEGIN(string);
<string>
{
    [^"]*        { yylval.sval = strdup (yytext); }
    \"           { BEGIN(INITIAL); return STRING; }
}

" "|\t           { continue; }
\n               { yycolumn = 1; continue; }
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
"*"              { return TIMES; }
"/"              { return DIVIDE; }
"="              { return EQ; }
"=="             { return EQEQ; }
"<>"             { return NEQ; }
"<"              { return LT; }
"<="             { return LE; }
">"              { return GT; }
">="             { return GE; }
"&"              { return AND; }
"|"              { return OR; }
{DIGIT}+         { yylval.ival=atoi(yytext); return INT; }
fn               { return FN; }
macro            { return MACRO; }
ret              { return RET; }
let              { return LET; }
def              { return DEF; }
if               { return IF; }
then             { return THEN; }
else             { return ELSE; }
while            { return WHILE; }
for              { return FOR; }
to               { return TO; }
do               { return DO; }
in               { return IN; }
end              { return END; }
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
{ID}             { yylval.sval = strdup (yytext); return ID;}
.                {
                    Vector_PushBack(&module->errors.lexer,
                        Error_New(
                            &yylloc,
                            1000,
                            "Unknown token %s",
                            yylloc.token));
                 }
