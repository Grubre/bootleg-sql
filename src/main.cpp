#include <cmath>
#include <fmt/base.h>

#include "GrammarLexer.h"
#include "GrammarParser.h"
#include "GrammarBaseVisitor.h"

class CalculatorInterpreter : public GrammarBaseVisitor {
public:
    std::any visitStart(GrammarParser::StartContext *ctx) override {
        try {
            return visit(ctx->expression());
        } catch (const std::exception& e) {
            fmt::println("Runtime error: {}", e.what());
            return std::numeric_limits<double>::quiet_NaN();
        }
    }

    std::any visitNumber(GrammarParser::NumberContext *ctx) override {
        try {
            return std::stod(ctx->NUMBER()->getText());
        } catch (const std::out_of_range&) {
            fmt::println("Error: Number out of range: {}", ctx->NUMBER()->getText());
            return std::numeric_limits<double>::quiet_NaN();
        } catch (const std::invalid_argument &) {
            fmt::println("Error: Invalid number format: {}", ctx->NUMBER()->getText());
            return std::numeric_limits<double>::quiet_NaN();
        }
    }

    std::any visitNegation(GrammarParser::NegationContext *ctx) override {
        double right = std::any_cast<double>(visit(ctx->right));
        return -right;
    }

    std::any visitParentheses(GrammarParser::ParenthesesContext *ctx) override {
        return visit(ctx->inner); // Evaluate the inner expression
    }

    std::any visitPower(GrammarParser::PowerContext *ctx) override {
        double left = std::any_cast<double>(visit(ctx->left));
        double right = std::any_cast<double>(visit(ctx->right));
        return std::pow(left, right);
    }

    std::any visitMultiplicationOrDivision(GrammarParser::MultiplicationOrDivisionContext *ctx) override {
        double left = std::any_cast<double>(visit(ctx->left));
        double right = std::any_cast<double>(visit(ctx->right));

        if (ctx->operator_->getType() == GrammarParser::STAR) {
            return left * right;
        } else { // SLASH
            if (right == 0.0) {
                std::cerr << "Error: Division by zero." << std::endl;
                return std::numeric_limits<double>::quiet_NaN();
            }
            return left / right;
        }
    }

    std::any visitAdditionOrSubtraction(GrammarParser::AdditionOrSubtractionContext *ctx) override {
        double left = std::any_cast<double>(visit(ctx->left));
        double right = std::any_cast<double>(visit(ctx->right));

        if (ctx->operator_->getType() == GrammarParser::PLUS) {
            return left + right;
        } else { // MINUS
            return left - right;
        }
    }
};

int main() {
    std::string line;
    CalculatorInterpreter interpreter;

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
        antlr4::tree::ParseTree *tree = parser.start();
        std::any result_any = interpreter.visit(tree);

        try {
            double result = std::any_cast<double>(result_any);
            if (std::isnan(result)) {
                fmt::println("Calculation resulted in an error.");
            } else {
                fmt::println("{}", result);
            }
        } catch(const std::bad_any_cast& e) {
            fmt::println("Error processing result.");
        }

        fmt::print("> ");
    }
    return 0;
}
