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

struct NamedTable {
    std::string name;
};
using TableOrSubquery = std::variant<NamedTable>;

struct SelectStmt {
    SelectModifier modifier;
    std::vector<ResultColumn> projections;
    std::vector<TableOrSubquery> sources;
};

using Statement = std::variant<SelectStmt>;

