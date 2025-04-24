#include <any>
#include <cmath>
#include <fmt/base.h>
#include <fmt/format.h>
#include <optional>
#include "expected.hpp"

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
        fail("Invalid SQL statement '{}'", ctx->toString());
    }

    std::any visitSelect_stmt(GrammarParser::Select_stmtContext *ctx) override {
        const auto modifier =
              ctx->ALL()      ? SelectModifier::ALL
            : ctx->DISTINCT() ? SelectModifier::DISTINCT
                              : SelectModifier::NONE;

        auto projections = std::vector<ResultColumn>{};
        projections.reserve(ctx->result_column().size());
        for (auto* result_column : ctx->result_column())
            projections.emplace_back(std::any_cast<ResultColumn>(visit(result_column)));

        auto sources = std::vector<TableOrSubquery>{};
        sources.reserve(ctx->table_or_subquery().size());
        for (auto* table_or_subquery : ctx->table_or_subquery())
            sources.emplace_back(std::any_cast<TableOrSubquery>(visit(table_or_subquery)));

        return SelectStmt {
            .modifier = modifier,
            .projections = std::move(projections),
            .sources = std::move(sources)
        };
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

    std::any visitTable_or_subquery(GrammarParser::Table_or_subqueryContext *ctx) override {
        return TableOrSubquery{};
        return visit(ctx->table_name());
    }

    std::any visitExpr(GrammarParser::ExprContext *ctx) override {
        return Expr{ctx->IDENTIFIER()->getText()};
    }

    std::any visitColumn_alias(GrammarParser::Column_aliasContext *ctx) override {
        return ctx->IDENTIFIER()->getText();
    }

    std::any visitTable_name(GrammarParser::Table_nameContext *ctx) override {
        return ctx->IDENTIFIER()->getText();
    }

    std::any visitColumn_name(GrammarParser::Column_nameContext *ctx) override {
        return ctx->IDENTIFIER()->getText();
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
