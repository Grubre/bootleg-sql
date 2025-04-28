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

int main(int argc, char** argv) {
    if (argc != 2) {
        fmt::println("Usage: {} <input query>", argv[0]);
        return 1;
    }

    std::string line = argv[1];
    SqlGrammarVisitor IR_generator;

    fmt::println("Input: {}", line);

    antlr4::ANTLRInputStream input(line);
    GrammarLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    GrammarParser parser(&tokens);
    antlr4::tree::ParseTree *tree = parser.program();

    try {
        auto result_any = IR_generator.visit(tree);
        auto statement = std::any_cast<Statement>(result_any);
        fmt::println("{}", to_string(statement));
    } catch(const SqlError& e) {
        fmt::println(stderr, "{}", e.what());
    }
    return 0;
}
