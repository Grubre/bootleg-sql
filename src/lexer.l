%option bison
%option noyywrap

%{
#include "parser.hpp"
%}

%%

[ \t\r\n]+       ;

[0-9]+           { yylval = atoi(text()); return NUMBER; }
"+"              { return PLUS; }
"*"              { return TIMES; }
"("              { return LPAREN; }
")"              { return RPAREN; }

.                { printf("Unknown character: %s\n", text()); }

%%
