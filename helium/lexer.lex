%{
    #include <string.h>

    #include "parser.h"
    #include "errormsg.h"

    #define YY_USER_ACTION adjust();

    int charPos = 1;

    int yywrap (void)
    {
        charPos = 1;
        return 1;
    }

    void adjust (void)
    {
        EM_tokPos = charPos;
        charPos += yyleng;
    }
%}

%x comment
%x string

DIGIT    [0-9]
ID       [_a-zA-Z][_a-zA-Z0-9]*

%%

"/*"             BEGIN(comment);
<comment>
{
    [^*\n]*         /* eat anything that's not a '*' */
    "*"+[^*/\n]*    /* eat up '*'s not followed by '/'s */
    "*"+"/"         BEGIN(INITIAL);
}
\"               BEGIN(string);
<string>
{
    [^"]*        { yylval.sval = strdup (yytext); }
    \"           { BEGIN(INITIAL); return STRING; }
}
" "|\t           { continue; }
\n	             { EM_newline(); continue; }
","	             { return COMMA; }
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
.	             { EM_error(EM_tokPos,"illegal token"); }
