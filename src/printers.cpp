#include "printers.hpp"
#include <fmt/format.h>
#include <fmt/ranges.h>

auto to_string(const Expr& expression) -> std::string { return expression.name; }

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
        [](const NamedTable& nt) {
            if (nt.alias) return fmt::format("{}{} AS {}", *nt.schema_name, nt.table_name, *nt.alias);
            return fmt::format("{}{}", *nt.schema_name, nt.table_name);
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

auto to_string(const Statement& statement) -> std::string {
    return std::visit(overloaded   {
        [](const SelectStmt& stmt) -> std::string { return to_string(stmt); }
    }, statement);
}
