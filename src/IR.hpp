#pragma once
#include <string>
#include <optional>
#include <variant>
#include <vector>

enum class SelectModifier {
    NONE,
    DISTINCT,
    ALL
};

struct Expr { std::string name; };

// https://sqlite.org/syntax/result-column.html
struct StarColumn { };
struct TableStarColumn { std::string table_name; };
struct ExprColumn {
    Expr expr;
    std::optional<std::string> alias;
};
using ResultColumn = std::variant<StarColumn, TableStarColumn, ExprColumn>;

struct Table {
    std::string table_name;
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

struct ColumnConstraint {

};

struct ColumnDef {
    std::string column_name;
    std::optional<std::string> type_name;
    std::vector<ColumnConstraint> column_constraints{};
};

enum class TableOption {
    WITHOUT_ROWID,
    STRICT
};

struct CreateTableStmt {
    bool is_temporary;
    bool if_not_exists_clause;
    Table table;
    std::vector<ColumnDef> column_definitions;
    std::vector<TableOption> table_options;
};

using Statement = std::variant<SelectStmt, CreateTableStmt>;

