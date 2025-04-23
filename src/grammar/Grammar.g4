// based on diagrams from https://www.sqlite.org/lang.html
grammar Grammar;

program : sql_stmt+;

sql_stmt
    : select_stmt
    ;

// https://www.sqlite.org/lang_select.html
select_stmt
    : SELECT (DISTINCT | ALL)? result_column (COMMA result_column)* FROM table_or_subquery (COMMA table_or_subquery)*
    ;

result_column
    : expr (AS column_alias)?
    | STAR
    | table_name DOT STAR
    ;

table_or_subquery
    : table_name
    ;

expr
    : IDENTIFIER
    ;

column_alias
    : IDENTIFIER
    ;

table_name
    : IDENTIFIER
    ;

column_name
    : IDENTIFIER
    ;

fragment DIGIT      : [0-9]     ;
fragment LETTER     : [a-zA-Z]  ;
fragment ID_CHAR    : (LETTER | DIGIT | '_' | '$') ;

IDENTIFIER
    : LETTER ID_CHAR*
    ;

SELECT : 'SELECT';
DISTINCT : 'DISTINCT';
ALL : 'ALL';
AS : 'AS';

COMMA : ',';
DOT : '.';
STAR : '*';

WHITESPACE: [ \r\n\t]+ -> skip;
