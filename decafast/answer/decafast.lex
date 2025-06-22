%{
#include "default-defs.h"
#include "decafast.tab.h"
#include <cstring>
#include <string>
#include <sstream>
#include <iostream>

using namespace std;

int lineno = 1;
int tokenpos = 1;

%}

DIGIT        [0-9]+
ESC_SEQ      \\[nrtvfab'\"\\]
CHAR_CONST   \'([^\\'\n]|{ESC_SEQ})\'
STRING_CONST \"([^\"\\\n]|{ESC_SEQ})*\"
ID           [a-zA-Z_][a-zA-Z_0-9]*
COMMENT      \/\/[^\n]*\n
WS_INLINE    [ \t\r\f\v]+
NL           \n


%%



\'\\\'                               { tokenpos+=yyleng; 
                                       cerr<<"unterminated char\n";
                                       return YYUNDEF; }
\'\'                                  { tokenpos+=yyleng;
                                       cerr<<"zero-width char\n";
                                       return YYUNDEF; }
\'([^\\'\n]|\\[nrtvfab'\"\\]){2,}\'   { tokenpos+=yyleng;
                                       cerr<<"char too long\n";
                                       return YYUNDEF; }
\'([^\\'\n]|\\[nrtvfab'\"\\])*\\[^nrtvfab'\"\\][^\\'\n]*\' { tokenpos+=yyleng;
                                       cerr<<"bad escape in char\n";
                                       return YYUNDEF; }
\'([^\\'\n]|\\[nrtvfab'\"\\])*$       { tokenpos+=yyleng;
                                       cerr<<"unterminated char\n";
                                       return YYUNDEF; }

\"([^\\\n]|\\[nrtvfab'\"\\])*\\[^nrtvfab'\"\\][^\"\\\n]*\" { tokenpos+=yyleng;
                                       cerr<<"bad escape in string\n";
                                       return YYUNDEF; }
\"[^\"\n]*\n                          { tokenpos+=yyleng; ++lineno; tokenpos=1;
                                       cerr<<"newline in string\n";
                                       return YYUNDEF; }
\"([^\\\"\n]|\\[nrtvfab'\"\\])*$      { tokenpos+=yyleng;
                                       cerr<<"unterminated string\n";
                                       return YYUNDEF; }


func            { tokenpos+=yyleng; return T_FUNC; }
int             { tokenpos+=yyleng; return T_INTTYPE; }
package         { tokenpos+=yyleng; return T_PACKAGE; }
extern          { tokenpos+=yyleng; return T_EXTERN; }
var             { tokenpos+=yyleng; return T_VAR; }
void            { tokenpos+=yyleng; return T_VOID; }
return          { tokenpos+=yyleng; return T_RETURN; }
string          { tokenpos+=yyleng; return T_STRINGTYPE; }
bool            { tokenpos+=yyleng; return T_BOOLTYPE; }
break           { tokenpos+=yyleng; return T_BREAK; }
continue        { tokenpos+=yyleng; return T_CONTINUE; }
if              { tokenpos+=yyleng; return T_IF; }
else            { tokenpos+=yyleng; return T_ELSE; }
for             { tokenpos+=yyleng; return T_FOR; }
while           { tokenpos+=yyleng; return T_WHILE; }
true            { tokenpos+=yyleng; return T_TRUE; }
false           { tokenpos+=yyleng; return T_FALSE; }
null            { tokenpos+=yyleng; return T_NULL; }


"=="   { tokenpos+=yyleng; return T_EQ; }
"!="   { tokenpos+=yyleng; return T_NEQ; }
"&&"   { tokenpos+=yyleng; return T_AND; }
"||"   { tokenpos+=yyleng; return T_OR; }
">="   { tokenpos+=yyleng; return T_GEQ; }
"<="   { tokenpos+=yyleng; return T_LEQ; }
">>"   { tokenpos+=yyleng; return T_RIGHTSHIFT; }
"<<"   { tokenpos+=yyleng; return T_LEFTSHIFT; }
"+"    { tokenpos+=yyleng; return T_PLUS; }
"-"    { tokenpos+=yyleng; return T_MINUS; }
"*"    { tokenpos+=yyleng; return T_MULT; }
"/"    { tokenpos+=yyleng; return T_DIV; }
"%"    { tokenpos+=yyleng; return T_MOD; }
"<"    { tokenpos+=yyleng; return T_LT; }
">"    { tokenpos+=yyleng; return T_GT; }
"="    { tokenpos+=yyleng; return T_ASSIGN; }
"."    { tokenpos+=yyleng; return T_DOT; }
";"    { tokenpos+=yyleng; return T_SEMICOLON; }
","    { tokenpos+=yyleng; return T_COMMA; }
"{"    { tokenpos+=yyleng; return T_LCB; }
"}"    { tokenpos+=yyleng; return T_RCB; }
"("    { tokenpos+=yyleng; return T_LPAREN; }
")"    { tokenpos+=yyleng; return T_RPAREN; }
"["    { tokenpos+=yyleng; return T_LSB; }
"]"    { tokenpos+=yyleng; return T_RSB; }
"!"    { tokenpos+=yyleng; return T_NOT; }



{DIGIT}        { yylval.ival = atoi(yytext);          tokenpos+=yyleng; return T_INTCONSTANT; }
{CHAR_CONST}   { yylval.cval = yytext[1];             tokenpos+=yyleng; return T_CHARCONSTANT; }
{STRING_CONST} { yylval.sval = new string(yytext);    tokenpos+=yyleng; return T_STRINGCONSTANT; }
{ID}           { yylval.sval = new string(yytext);    tokenpos+=yyleng; return T_ID; }


{COMMENT}      { ++lineno; tokenpos = 1; }
{WS_INLINE}    { tokenpos+=yyleng; }
{NL}           { ++lineno; tokenpos = 1; }

. { std::cerr<<lineno<<":"<<tokenpos<<" bad char '"<<yytext[0]<<"'\n";
    ++tokenpos; return YYUNDEF; }

%%  


int yyerror(const char *s) {
  cerr << lineno << ": " << s << " at char " << tokenpos << endl;
  return 1;
}

