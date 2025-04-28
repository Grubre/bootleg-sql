#pragma once
#include "GrammarBaseVisitor.h"
#include <fmt/base.h>
#include <fmt/format.h>
#include <stdexcept>
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

// ===================================
// IR generator
// ===================================

struct SqlError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

template <typename... Args>
[[noreturn]] void fail(fmt::format_string<Args...> fmtstr, Args&&... args) {
    throw SqlError{fmt::format(fmtstr, std::forward<Args>(args)...)};
}

class SqlGrammarVisitor : public GrammarBaseVisitor {
public:
    std::any visitProgram(GrammarParser::ProgramContext *ctx) override;
    std::any visitSql_stmt(GrammarParser::Sql_stmtContext *ctx) override;
    std::any visitInsert_stmt(GrammarParser::Insert_stmtContext *ctx) override;
    std::any visitSelect_stmt(GrammarParser::Select_stmtContext *ctx) override;
    std::any visitCreate_table_stmt(GrammarParser::Create_table_stmtContext *ctx) override;
    std::any visitColumn_def(GrammarParser::Column_defContext *ctx) override;
    std::any visitColumn_constraint([[maybe_unused]] GrammarParser::Column_constraintContext *ctx) override;
    std::any visitResult_column(GrammarParser::Result_columnContext *ctx) override;
    std::any visitTable_or_subquery(GrammarParser::Table_or_subqueryContext *ctx) override;
    std::any visitCommon_table_expression(GrammarParser::Common_table_expressionContext *ctx) override;
    std::any visitWith_clause(GrammarParser::With_clauseContext *ctx) override;
    std::any visitConfilct_resolution_method(GrammarParser::Confilct_resolution_methodContext *ctx) override;
    std::any visitExpr(GrammarParser::ExprContext *ctx) override;
    std::any visitColumn_alias(GrammarParser::Column_aliasContext *ctx) override;
    std::any visitType_name(GrammarParser::Type_nameContext *ctx) override;
    std::any visitSchema_name(GrammarParser::Schema_nameContext *ctx) override;
    std::any visitTable_alias(GrammarParser::Table_aliasContext *ctx) override;
    std::any visitTable_name(GrammarParser::Table_nameContext *ctx) override;
    std::any visitColumn_name(GrammarParser::Column_nameContext *ctx) override;

private:
    // used to collect grammar constructs like "column_name (COMMA column_name)*" that antlr parses into vec
    // U - some grammar rule, hence the requires clause
    // T - the IR struct representing that grammar rule, returned by the corresponding, overriden visit function
    template <typename T, typename U>
        requires std::derived_from<std::remove_pointer_t<U>, antlr4::ParserRuleContext>
    auto collect(const std::vector<U>& grammar_expr) -> std::vector<T> {
        auto vec = std::vector<T>{};
        vec.reserve(grammar_expr.size());
        for (const auto& e : grammar_expr) {
            vec.push_back(std::any_cast<T>(visit(e)));
        }

        return vec;
    }
};
