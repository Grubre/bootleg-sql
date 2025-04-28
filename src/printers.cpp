#include "printers.hpp"
#include "IR.hpp"
#include <fmt/format.h>
#include <fmt/ranges.h>
#include "common.hpp"

auto conflict_method_to_string(ConflictResolutionMethod method) -> std::string {
    switch (method) {
        case ConflictResolutionMethod::ABORT:    return "ABORT";
        case ConflictResolutionMethod::FAIL:     return "FAIL";
        case ConflictResolutionMethod::IGNORE:   return "IGNORE";
        case ConflictResolutionMethod::REPLACE:  return "REPLACE";
        case ConflictResolutionMethod::ROLLBACK: return "ROLLBACK";
        default: return "[UNKNOWN CONFLICT METHOD]";
    }
}

auto to_string(const Expr& expression) -> std::string { return expression.name; }

auto to_string(const Table& table) -> std::string {
    return fmt::format("{}{}", table.schema_name ? *table.schema_name : "", table.table_name);
}

auto to_string(const ResultColumn& rc) -> std::string {
    return std::visit(overloaded{
        [](const StarColumn&)               { return std::string{"*"}; },
        [](const TableStarColumn& tsc)      { return fmt::format("{}.{}", tsc.table_name, "*"); },
        [](const ExprColumn& ec) {
            if (ec.alias) return fmt::format("{} AS {}", to_string(ec.expr), *ec.alias);
            return to_string(ec.expr);
        }
    }, rc);
}

auto to_string(const TableOrSubquery& ts) -> std::string {
    return std::visit(overloaded{
        [](const AliasedTable& nt) {
            if (nt.alias) return fmt::format("{} AS {}", to_string(nt.table), *nt.alias);
            return fmt::format("{}", to_string(nt.table));
        }
    }, ts);
}

auto to_string(const SelectStmt& statement) -> std::string {
    const auto modifier_str =
        (statement.modifier == SelectModifier::ALL)      ? "ALL "
      : (statement.modifier == SelectModifier::DISTINCT) ? "DISTINCT "
      : /*(statement.modifier == SelectModifier::NONE)*/   "";

    std::vector<std::string> proj_strings;
    proj_strings.reserve(statement.projections.size());
    for (const auto& rc : statement.projections) proj_strings.push_back(to_string(rc));

    std::vector<std::string> src_strings;
    src_strings.reserve(statement.sources.size());
    for (const auto& s : statement.sources) src_strings.push_back(to_string(s));

    return fmt::format("SELECT {}{} FROM {}", modifier_str, fmt::join(proj_strings,", "), fmt::join(src_strings,", "));
}

auto to_string(const ColumnDef& def) -> std::string {
    const auto& type_str = def.type_name ? *def.type_name : "";

    // TODO: Finish the formatting here

    return def.column_name;
}

auto to_string(const CreateTableStmt& statement) -> std::string {
    const auto is_temporary_str = statement.temporary ? " TEMPORARY " : " ";
    const auto if_not_exists_str = statement.if_not_exists_clause ? "IF NOT EXISTS " : "";

    const auto table_str = to_string(statement.table);

    std::vector<std::string> column_def_strs{};
    column_def_strs.reserve(statement.column_definitions.size());
    for (const auto& s : statement.column_definitions) column_def_strs.push_back(to_string(s));

    // TODO: Finish the formatting here

    return fmt::format("CREATE TABLE{}{}{}({})", is_temporary_str, if_not_exists_str, table_str, fmt::join(column_def_strs, ", "));
}

auto to_string(const WithClause& with_clause) -> std::string {
    // TODO: Implement this
    return "";
}

auto to_string(const InsertStmt& statement) -> std::string {
    std::string result = "";

    if (statement.with_clause) {
        result += to_string(*statement.with_clause);
    }

    result += std::visit(overloaded{
        [](const ReplaceContainer&) {
            return std::string("REPLACE");
        },
        [&](const InsertContainer& ic) {
            std::string op_str = "INSERT";
            if (ic.confilct_res_method) {
                op_str += fmt::format(" OR {}", conflict_method_to_string(*ic.confilct_res_method));
            }
            return op_str;
        }
    }, statement.operation);

    result += fmt::format(" INTO {}", to_string(statement.table));

    if (statement.column_names && !statement.column_names->empty()) {
         result += fmt::format(" ({})", fmt::join(*statement.column_names, ", "));
    }

    result += " ";

    result += std::visit(overloaded{
        [](const DefaultValues&) -> std::string {
            return "DEFAULT VALUES";
        },
        [](const InsertStmtValuesExpr& values) -> std::string {
            std::vector<std::string> expr_strings;
            expr_strings.reserve(values.expressions.size());
            for (const auto& expr : values.expressions) {
                expr_strings.push_back(to_string(expr));
            }
            return fmt::format("VALUES({})", fmt::join(expr_strings, ", "));
        },
        [](const SelectStmt& select_stmt) -> std::string {
            return to_string(select_stmt);
        }
    }, statement.tuples);

    return result;
}
auto to_string(const Statement& statement) -> std::string {
    return std::visit(overloaded   {
        [](const SelectStmt& stmt) -> std::string { return to_string(stmt); },
        [](const CreateTableStmt& stmt) -> std::string { return to_string(stmt); },
        [](const InsertStmt& stmt) -> std::string { return to_string(stmt); }
    }, statement);
}
