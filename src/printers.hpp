#pragma once
#include "IR.hpp"

template <class... Ts> struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

[[nodiscard]] auto to_string(const SelectStmt& statement) -> std::string;
[[nodiscard]] auto to_string(const CreateTableStmt& statement) -> std::string;
[[nodiscard]] auto to_string(const Statement& statement) -> std::string;
[[nodiscard]] auto to_string(const Expr& expression) -> std::string;
[[nodiscard]] auto to_string(const ResultColumn& rc) -> std::string;
[[nodiscard]] auto to_string(const TableOrSubquery& ts) -> std::string;
