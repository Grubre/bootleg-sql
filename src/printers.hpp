#pragma once
#include "IR.hpp"

[[nodiscard]] auto to_string(const SelectStmt& statement) -> std::string;
[[nodiscard]] auto to_string(const CreateTableStmt& statement) -> std::string;
[[nodiscard]] auto to_string(const Statement& statement) -> std::string;
[[nodiscard]] auto to_string(const Expr& expression) -> std::string;
[[nodiscard]] auto to_string(const ResultColumn& rc) -> std::string;
[[nodiscard]] auto to_string(const TableOrSubquery& ts) -> std::string;
