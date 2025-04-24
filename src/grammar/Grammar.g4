// based on diagrams from https://www.sqlite.org/lang.html
grammar Grammar;
options { caseSensitive=false; }

program : sql_stmt ;

sql_stmt
    : select_stmt SEMI?
    ;

// https://www.sqlite.org/lang_select.html
select_stmt
    : SELECT (DISTINCT | ALL)? result_column (COMMA result_column)* FROM table_or_subquery (COMMA table_or_subquery)*
    ;

// https://sqlite.org/syntax/result-column.html
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

SELECT : 'SELECT';
DISTINCT : 'DISTINCT';
ALL : 'ALL';
AS : 'AS';
FROM : 'FROM';

COMMA : ',';
DOT : '.';
STAR : '*';
SEMI : ';';

IDENTIFIER
    : LETTER ID_CHAR*
    ;

WHITESPACE: [ \r\n\t]+ -> skip;
