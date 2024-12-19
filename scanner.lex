%{ /* Declarations section in C*/

#include "output.hpp"
#include "parser.tab.h"

%}

%option yylineno
%option noyywrap

%%

void                    { return VOID; }
int                     { return INT; }
byte                    { return BYTE; }
bool                    { return BOOL; }
and                     { return AND; }
or                      { return OR; }
not                     { return NOT; }
true                    { return TRUE; }
false                   { return FALSE; }
return                  { return RETURN; }
if                      { return IF; }
else                    { return ELSE; }
while                   { return WHILE; }
break                   { return BREAK; }
continue                { return CONTINUE; }
;                       { return SC; }
,                       { return COMMA; }
\(                      { return LPAREN; }
\)                      { return RPAREN; }
\{                      { return LBRACE; }
\}                      { return RBRACE; }
\=                      { return ASSIGN; }
"<"                     { return LT; }
"<="                    { return LE; }
">"                     { return GT; }
">="                    { return GE; }
"=="                    { return EQ; }
"!="                    { return NE; }
"+"                     { return ADD; }
"-"                     { return SUB; }
"*"                     { return MUL; }
"/"                     { return DIV; }
[ \t\n\r]+              { /* Ignore whitespace */ }
\/\/[^\r\n]*[\r|\n|\r\n]?          { /* Ignore comments */ }
[a-zA-Z][a-zA-Z0-9]*   {   yylval = std::make_shared<ast::ID>(yytext);
                            return ID; }

0|[1-9][0-9]*         {  yylval = std::make_shared<ast::Num>(yytext); return NUM; }
0b|[1-9][0-9]*b       {   std::string num_str(yytext);
                            num_str.pop_back();
                            yylval = std::make_shared<ast::NumB>(num_str.c_str()); return NUM_B;  }

\"([^\n\r\"\\]|\\[rnt\"\\])+\"   {  yylval = std::make_shared<ast::String>(yytext);
                                    return STRING; }

. {
    // printf("DEBUG: Matched wildcard rule: %s\n", yytext);
    output::errorLex(yylineno);
    exit(0);
}

%%