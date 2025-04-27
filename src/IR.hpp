#pragma once
#include <string>
#include <optional>
#include <variant>
#include <vector>

// ===================================
// SELECT STATEMENT
// ===================================
enum class SelectModifier {
    NONE,
    DISTINCT,
    ALL
};

using ColumnName = std::string;
using TableName = std::string;

struct Expr { std::string name; };

// https://sqlite.org/syntax/result-column.html
struct StarColumn { };
struct TableStarColumn { TableName table_name; };
struct ExprColumn {
    Expr expr;
    std::optional<std::string> alias;
};
using ResultColumn = std::variant<StarColumn, TableStarColumn, ExprColumn>;

struct Table {
    TableName table_name;
    std::optional<std::string> schema_name;
};
struct AliasedTable {
    Table table;
    std::optional<std::string> alias;
};
using TableOrSubquery = std::variant<AliasedTable>;

struct SelectStmt {
    SelectModifier modifier;
    std::vector<ResultColumn> projections;
    std::vector<TableOrSubquery> sources;
};

// ===================================
// CREATE TABLE STATEMENT
// ===================================
enum class ConflictResolutionMethod {
    ABORT,
    FAIL,
    IGNORE,
    REPLACE,
    ROLLBACK
};

struct ColumnConstraint {

};

struct ColumnDef {
    ColumnName column_name;
    std::optional<std::string> type_name;
    std::vector<ColumnConstraint> column_constraints{};
};

enum class TableOption {
    WITHOUT_ROWID,
    STRICT
};

struct CreateTableStmt {
    bool temporary;
    bool if_not_exists_clause;
    Table table;
    std::vector<ColumnDef> column_definitions;
    std::vector<TableOption> table_options;
};

// ===================================
// INSERT STATEMENT
// ===================================
enum class MateralizedSpecifier {
    MATERLIAZED,
    NOT_MATERIALIZED,
    NONE
};
struct CommonTableExpression {
    TableName name;
    std::vector<ColumnName> column_names;
    MateralizedSpecifier materliazed_specifier;
    SelectStmt select_stmt;
};

struct WithClause {
    bool recursive;
    std::vector<CommonTableExpression> common_table_expressions;
};

struct ReplaceContainer {};
struct InsertContainer {std::optional<ConflictResolutionMethod> confilct_res_method;};
using InsertStmtOp = std::variant<ReplaceContainer, InsertContainer>;

// this struct represents use of the VALUES keyword, e.g. INSERT INTO temp VALUES("123", 5)
struct InsertStmtValuesExpr {
    std::vector<Expr> expressions;
};
struct DefaultValues {};
using InsertedTuples = std::variant<InsertStmtValuesExpr, SelectStmt, DefaultValues>;

struct InsertStmt {
    std::optional<WithClause> with_clause;
    InsertStmtOp operation;
    AliasedTable table;
    std::optional<std::vector<ColumnName>> column_names;
    InsertedTuples tuples;
};

using Statement = std::variant<SelectStmt, CreateTableStmt, InsertStmt>;

