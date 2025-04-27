// based on diagrams from https://www.sqlite.org/lang.html
// https://github.com/antlr/grammars-v4/blob/master/sql/sqlite/SQLiteParser.g4
// https://github.com/antlr/grammars-v4/blob/master/sql/sqlite/SQLiteLexer.g4
grammar Grammar;
options {
    caseInsensitive = true;
}

program : SEMI* sql_stmt SEMI* ;

sql_stmt
    : select_stmt
    | create_table_stmt
    | insert_stmt
    ;

// TODO: Fully implement https://sqlite.org/lang_insert.html
insert_stmt
    : with_clause? (REPLACE | (INSERT (OR confilct_resolution_method)?)) INTO (schema_name DOT)? table_name (AS table_alias)?
        (LPAREN column_name (COMMA column_name)* RPAREN)?
        ( (VALUES LPAREN expr (COMMA expr)* RPAREN)
        | select_stmt
        | (DEFAULT VALUES)
        )
    ;

confilct_resolution_method
    : (ABORT | FAIL | IGNORE | REPLACE | ROLLBACK)
    ;

with_clause
    : WITH RECURSIVE? common_table_expression (COMMA common_table_expression)
    ;

// https://sqlite.org/syntax/common-table-expression.html
common_table_expression
    : table_name (LPAREN column_name (COMMA column_name)* RPAREN)? AS (NOT? MATERIALIZED)? LPAREN select_stmt RPAREN
    ;

// https://sqlite.org/lang_createtable.html
create_table_stmt
    : CREATE (TEMP | TEMPORARY)? TABLE (IF NOT EXISTS)? (schema_name DOT)? table_name ((AS select_stmt) | (LPAREN column_def (COMMA column_def)* RPAREN table_options?))
    ;

column_def : column_name type_name? column_constraint* ;

type_name
    : IDENTIFIER
    ;

// TODO: fully implement https://sqlite.org/syntax/column-constraint.html
column_constraint
    : NOT NULL conflict_clause
    ;

conflict_clause
    : (ON CONFLICT confilct_resolution_method)?
    ;


table_options
    : WITHOUT ROWID
    | STRICT
    | COMMA table_options
    ;

// TODO: Implement the rest of https://www.sqlite.org/lang_select.html
select_stmt
    : SELECT (DISTINCT | ALL)? result_column (COMMA result_column)* FROM table_or_subquery (COMMA table_or_subquery)*
    ;

// https://sqlite.org/syntax/result-column.html
result_column
    : expr (AS column_alias)?
    | STAR
    | table_name DOT STAR
    ;

// TODO: Implement the rest of https://sqlite.org/syntax/table-or-subquery.html
table_or_subquery
    : (schema_name DOT)? table_name (AS table_alias)?
    ;

// TODO: Implement the rest of https://sqlite.org/syntax/expr.html
expr
    : IDENTIFIER
    ;

table_alias
    : IDENTIFIER
    ;

schema_name
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

CREATE : 'CREATE';
TEMP : 'TEMP';
TEMPORARY : 'TEMPORARY';
TABLE : 'TABLE';
IF : 'IF';
NOT : 'NOT';
EXISTS : 'EXISTS';
ON : 'ON';
CONFLICT : 'CONFLICT';
ROLLBACK : 'ROLLBACK';
ABORT : 'ABORT';
FAIL : 'FAIL';
IGNORE : 'IGNORE';
REPLACE : 'REPLACE';
NULL : 'NULL';
WITHOUT : 'WITHOUT';
ROWID : 'ROWID';
MATERIALIZED : 'MATERIALIZED';
WITH : 'WITH';
RECURSIVE : 'RECURSIVE';
STRICT : 'STRICT';

INSERT : 'INSERT';
INTO : 'INTO';
VALUES : 'VALUES';
DEFAULT : 'DEFAULT';
OR : 'OR';

LPAREN : '(';
RPAREN : ')';
COMMA : ',';
DOT : '.';
STAR : '*';
SEMI : ';';

IDENTIFIER
    : LETTER ID_CHAR*
    ;

WHITESPACE: [ \r\n\t]+ -> skip;
