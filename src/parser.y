%{
#include <cstdio>
#include <cstdlib>

void yyerror(const char* s);
int yylex();
%}

%token NUMBER
%token PLUS TIMES LPAREN RPAREN

%left PLUS
%left TIMES

%%

input:
    expr '\n' { printf("Result: %d\n", $1); }
  | expr      { printf("Result: %d\n", $1); }
  ;

expr:
    expr PLUS expr   { $$ = $1 + $3; }
  | expr TIMES expr  { $$ = $1 * $3; }
  | LPAREN expr RPAREN { $$ = $2; }
  | NUMBER           { $$ = $1; }
  ;

%%
