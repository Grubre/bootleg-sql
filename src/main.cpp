#include <cmath>
#include <fmt/base.h>
#include <fmt/format.h>
#include "expected.hpp"

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
using ProgramOutput = std::vector<SqlBytecodeProgram>;

struct SqlError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

template <typename... Args>
[[noreturn]] void fail(fmt::format_string<Args...> fmtstr, Args&&... args) {
    throw SqlError{fmt::format(fmtstr, std::forward<Args>(args)...)};
}

enum class SelectModifier {
    NONE,
    DISTINCT,
    ALL
};

class SqlGrammarVisitor : public GrammarBaseVisitor {
public:
    std::any visitProgram(GrammarParser::ProgramContext *ctx) override {
        auto programs = std::vector<SqlBytecodeProgram>(ctx->children.size());

        for (auto i = 0u; i < ctx->children.size(); i++) {
            programs[i] = std::any_cast<SqlBytecodeProgram>(visit(ctx->children[i]));
        }

        return programs;
    }

    std::any visitSql_stmt(GrammarParser::Sql_stmtContext *ctx) override {
        if (ctx->select_stmt()) {
            return visit(ctx->select_stmt());
        }
        fail("Invalid SQL statement '{}'", ctx->toString());
    }

    std::any visitSelect_stmt(GrammarParser::Select_stmtContext *ctx) override {
        const auto modifier = ctx->ALL() ? SelectModifier::ALL : (ctx->DISTINCT() ? SelectModifier::DISTINCT : SelectModifier::NONE);
        return nullptr;
    }

    std::any visitResult_column(GrammarParser::Result_columnContext *ctx) override {
        if (ctx->expr()) {
            return visit(ctx->expr());
        }
        fail("Invalid result column value");
    }

    std::any visitTable_or_subquery(GrammarParser::Table_or_subqueryContext *ctx) override {
        return visit(ctx->table_name());
    }

    std::any visitExpr(GrammarParser::ExprContext *ctx) override {
        return ctx->IDENTIFIER();
    }

    std::any visitColumn_alias(GrammarParser::Column_aliasContext *ctx) override {
        return ctx->IDENTIFIER();
    }

    std::any visitTable_name(GrammarParser::Table_nameContext *ctx) override {
        return ctx->IDENTIFIER();
    }

    std::any visitColumn_name(GrammarParser::Column_nameContext *ctx) override {
        return ctx->IDENTIFIER();
    }
};

int main() {
    std::string line;
    SqlGrammarVisitor bytecode_generator;

    fmt::print("> ");
    while (std::getline(std::cin, line)) {
        if (line.empty()) {
            fmt::print("> ");
            continue;
        }

        antlr4::ANTLRInputStream input(line);
        GrammarLexer lexer(&input);
        antlr4::CommonTokenStream tokens(&lexer);
        GrammarParser parser(&tokens);
        antlr4::tree::ParseTree *tree = parser.program();
        try {
            auto result_any = bytecode_generator.visit(tree);
            auto programs = std::any_cast<ProgramOutput>(result_any);
        } catch(const SqlError& e) {
            fmt::println(stderr, "{}", e.what());
        }

        fmt::print("> ");
    }
    return 0;
}
