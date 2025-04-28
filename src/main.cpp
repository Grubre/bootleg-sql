#include <any>
#include <cmath>
#include <concepts>
#include <fmt/base.h>
#include <fmt/format.h>
#include <optional>

#include "IR.hpp"
#include "printers.hpp"

#include "GrammarLexer.h"
#include "GrammarParser.h"
#include "GrammarBaseVisitor.h"

enum class Opcode {
    NOOP,
    HALT
};
enum class P4type {
    I32,
    I64,
    F64,
    STR,
    BLOB
};
struct Instruction {
    Opcode opcode;
    std::int32_t P1;
    std::int32_t P2;
    std::int32_t P3;
    void* P4;
    P4type p4type;
    std::uint16_t P5;
};

using SqlBytecodeProgram = std::vector<Instruction>;
using ProgramOutput = SqlBytecodeProgram;

struct SqlError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

template <typename... Args>
[[noreturn]] void fail(fmt::format_string<Args...> fmtstr, Args&&... args) {
    throw SqlError{fmt::format(fmtstr, std::forward<Args>(args)...)};
}

class SqlGrammarVisitor : public GrammarBaseVisitor {
public:
    std::any visitProgram(GrammarParser::ProgramContext *ctx) override {
        return visit(ctx->sql_stmt());
    }

    std::any visitSql_stmt(GrammarParser::Sql_stmtContext *ctx) override {
        if (ctx->select_stmt()) {
            return Statement{std::any_cast<SelectStmt>(visit(ctx->select_stmt()))};
        }
        if (ctx->create_table_stmt()) {
            return Statement{std::any_cast<CreateTableStmt>(visit(ctx->create_table_stmt()))};
        }
        if (ctx->insert_stmt()) {
            return Statement{std::any_cast<InsertStmt>(visit(ctx->insert_stmt()))};
        }
        fail("Invalid SQL statement '{}'", ctx->getText());
    }

    std::any visitInsert_stmt(GrammarParser::Insert_stmtContext *ctx) override {
        auto with_clause = std::optional<WithClause>{};
        if (ctx->with_clause()) {
            with_clause = std::any_cast<WithClause>(visit(ctx->with_clause()));
        }

        auto operation = InsertStmtOp{};
        if (ctx->INSERT()) {
            operation = InsertContainer{};
            if (ctx->confilct_resolution_method()) {
                std::get<InsertContainer>(operation).confilct_res_method =
                    std::any_cast<ConflictResolutionMethod>(visit(ctx->confilct_resolution_method()));
            }
        } else if (ctx->REPLACE()) {
            operation = ReplaceContainer {};
        }

        auto schema_name = std::optional<std::string>{};
        if (ctx->schema_name()) { schema_name = std::any_cast<std::string>(visit(ctx->schema_name())); }
        auto alias = std::optional<std::string>{};
        if (ctx->table_alias()) { alias = std::any_cast<std::string>(visit(ctx->table_alias())); }
        auto aliased_table = AliasedTable {
            .table = Table {
                .table_name = std::any_cast<TableName>(visit(ctx->table_name())),
                .schema_name = std::move(schema_name)
            },
            .alias = std::move(alias)
        };

        auto tuples = InsertedTuples{DefaultValues{}};
        if (ctx->VALUES()) {
            tuples = InsertStmtValuesExpr{
                .expressions = collect<Expr>(ctx->expr())
            };
        } else if (ctx->select_stmt()) {
            tuples = std::any_cast<SelectStmt>(visit(ctx->select_stmt()));
        }

        return InsertStmt {
            .with_clause = std::move(with_clause),
            .operation = std::move(operation),
            .table = std::move(aliased_table),
            .column_names = collect<ColumnName>(ctx->column_name()),
            .tuples = std::move(tuples)
        };
    }

    std::any visitSelect_stmt(GrammarParser::Select_stmtContext *ctx) override {
        const auto modifier =
              ctx->ALL()      ? SelectModifier::ALL
            : ctx->DISTINCT() ? SelectModifier::DISTINCT
                              : SelectModifier::NONE;

        return SelectStmt {
            .modifier = modifier,
            .projections = collect<ResultColumn>(ctx->result_column()),
            .sources = collect<TableOrSubquery>(ctx->table_or_subquery())
        };
    }

    std::any visitCreate_table_stmt(GrammarParser::Create_table_stmtContext *ctx) override {
        const auto is_temporary = bool(ctx->TEMPORARY());
        const auto if_not_exists_clause = bool(ctx->IF());

        auto schema_name_opt = std::optional<std::string>{};
        if (ctx->schema_name()) {
            schema_name_opt = std::any_cast<std::string>(visit(ctx->schema_name()));
        }

        auto table = Table {
            .table_name = std::any_cast<std::string>(visit(ctx->table_name())),
            .schema_name = schema_name_opt
        };

        // TODO: Both here and in the struct, parse the `AS select_stmt` variant (https://sqlite.org/lang_createtable.html)

        return CreateTableStmt {
            .temporary = is_temporary,
            .if_not_exists_clause = if_not_exists_clause,
            .table = std::move(table),
            .column_definitions = collect<ColumnDef>(ctx->column_def()),
            .table_options = {}
        };
    }

    std::any visitColumn_def(GrammarParser::Column_defContext *ctx) override {
        auto type_name = std::optional<std::string>{};
        if (ctx->type_name()) {
            type_name = std::any_cast<std::string>(visit(ctx->type_name()));
        }

        return ColumnDef {
            .column_name = std::any_cast<std::string>(visit(ctx->column_name())),
            .type_name = std::move(type_name),
            .column_constraints = collect<ColumnConstraint>(ctx->column_constraint())
        };
    }

    std::any visitColumn_constraint([[maybe_unused]] GrammarParser::Column_constraintContext *ctx) override {
        // TODO: Implement this
        fail("Column constraints not yet implemented");
    }

    std::any visitResult_column(GrammarParser::Result_columnContext *ctx) override {
        if (ctx->table_name()) {
            return ResultColumn{TableStarColumn {.table_name = std::any_cast<std::string>(visit(ctx->table_name()))}};
        }
        if (ctx->STAR()) {
            return ResultColumn{StarColumn{}};
        }
        if (ctx->expr()) {
            std::optional<std::string> alias = std::nullopt;
            if (ctx->column_alias()) {
                alias = std::any_cast<std::string>(ctx->column_alias());
            }
            return ResultColumn{ExprColumn {
                .expr = std::any_cast<Expr>(visit(ctx->expr())),
                .alias = alias
            }};
        }
        fail("Invalid result column value");
    }

    // https://sqlite.org/syntax/table-or-subquery.html
    std::any visitTable_or_subquery(GrammarParser::Table_or_subqueryContext *ctx) override {
        auto table_or_subquery = AliasedTable{
            .table = Table {
                .table_name = std::any_cast<std::string>(visit(ctx->table_name())),
                .schema_name = std::nullopt,
            },
            .alias = std::nullopt
        };
        if (ctx->schema_name()) {
            table_or_subquery.table.schema_name = std::any_cast<std::string>(visit(ctx->schema_name()));
        }
        if (ctx->table_alias()) {
            table_or_subquery.alias = std::any_cast<std::string>(visit(ctx->table_alias()));
        }

        return TableOrSubquery{table_or_subquery};
    }

    std::any visitCommon_table_expression(GrammarParser::Common_table_expressionContext *ctx) override {
        const auto materliazed_specifier =
              ctx->NOT()          ? MateralizedSpecifier::NOT_MATERIALIZED
            : ctx->MATERIALIZED() ? MateralizedSpecifier::MATERLIAZED
            : /* Not specified */   MateralizedSpecifier::NONE;

        return CommonTableExpression {
            .name = std::any_cast<TableName>(visit(ctx->table_name())),
            .column_names = collect<ColumnName>(ctx->column_name()),
            .materliazed_specifier = materliazed_specifier,
            .select_stmt = std::any_cast<SelectStmt>(visit(ctx->select_stmt()))
        };
    }

    std::any visitWith_clause(GrammarParser::With_clauseContext *ctx) override {
        return WithClause {
            .recursive = bool(ctx->RECURSIVE()),
            .common_table_expressions = collect<CommonTableExpression>(ctx->common_table_expression())
        };
    }

    std::any visitConfilct_resolution_method(GrammarParser::Confilct_resolution_methodContext *ctx) override {
        if (ctx->ABORT())    return ConflictResolutionMethod::ABORT;
        if (ctx->FAIL())     return ConflictResolutionMethod::FAIL;
        if (ctx->IGNORE())   return ConflictResolutionMethod::IGNORE;
        if (ctx->REPLACE())  return ConflictResolutionMethod::REPLACE;
        if (ctx->ROLLBACK()) return ConflictResolutionMethod::ROLLBACK;

        fail("Unknown resolution method {}", ctx->getText());
    }

    std::any visitExpr(GrammarParser::ExprContext *ctx) override {
        return Expr{ctx->IDENTIFIER()->getText()};
    }

    std::any visitColumn_alias(GrammarParser::Column_aliasContext *ctx) override {
        return ctx->IDENTIFIER()->getText();
    }

    std::any visitType_name(GrammarParser::Type_nameContext *ctx) override {
        return ctx->IDENTIFIER()->getText();
    }

    std::any visitSchema_name(GrammarParser::Schema_nameContext *ctx) override {
        return ctx->IDENTIFIER()->getText();
    }

    std::any visitTable_alias(GrammarParser::Table_aliasContext *ctx) override {
        return ctx->IDENTIFIER()->getText();
    }

    std::any visitTable_name(GrammarParser::Table_nameContext *ctx) override {
        return ctx->IDENTIFIER()->getText();
    }

    std::any visitColumn_name(GrammarParser::Column_nameContext *ctx) override {
        return ctx->IDENTIFIER()->getText();
    }

private:
    // used to collect grammar constructs like "column_name (COMMA column_name)*" that antlr parses into vec
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

int main(int argc, char** argv) {
    if (argc != 2) {
        fmt::println("Usage: {} <input query>", argv[0]);
        return 1;
    }

    std::string line = argv[1];
    SqlGrammarVisitor bytecode_generator;

    fmt::println("Input: {}", line);

    antlr4::ANTLRInputStream input(line);
    GrammarLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    GrammarParser parser(&tokens);
    antlr4::tree::ParseTree *tree = parser.program();

    try {
        auto result_any = bytecode_generator.visit(tree);
        auto statement = std::any_cast<Statement>(result_any);
        fmt::println("{}", to_string(statement));
    } catch(const SqlError& e) {
        fmt::println(stderr, "{}", e.what());
    }
    return 0;
}
