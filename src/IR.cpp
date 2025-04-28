#include "IR.hpp"
std::any SqlGrammarVisitor::visitProgram(GrammarParser::ProgramContext *ctx) {
    return visit(ctx->sql_stmt());
}

std::any SqlGrammarVisitor::visitSql_stmt(GrammarParser::Sql_stmtContext *ctx) {
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

std::any SqlGrammarVisitor::visitInsert_stmt(GrammarParser::Insert_stmtContext *ctx) {
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

std::any SqlGrammarVisitor::visitSelect_stmt(GrammarParser::Select_stmtContext *ctx) {
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

std::any SqlGrammarVisitor::visitCreate_table_stmt(GrammarParser::Create_table_stmtContext *ctx) {
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

std::any SqlGrammarVisitor::visitColumn_def(GrammarParser::Column_defContext *ctx) {
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

std::any SqlGrammarVisitor::visitColumn_constraint([[maybe_unused]] GrammarParser::Column_constraintContext *ctx) {
    // TODO: Implement this
    fail("Column constraints not yet implemented");
}

std::any SqlGrammarVisitor::visitResult_column(GrammarParser::Result_columnContext *ctx) {
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

std::any SqlGrammarVisitor::visitTable_or_subquery(GrammarParser::Table_or_subqueryContext *ctx) {
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

std::any SqlGrammarVisitor::visitCommon_table_expression(GrammarParser::Common_table_expressionContext *ctx) {
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

std::any SqlGrammarVisitor::visitWith_clause(GrammarParser::With_clauseContext *ctx) {
    return WithClause {
        .recursive = bool(ctx->RECURSIVE()),
            .common_table_expressions = collect<CommonTableExpression>(ctx->common_table_expression())
    };
}

std::any SqlGrammarVisitor::visitConfilct_resolution_method(GrammarParser::Confilct_resolution_methodContext *ctx) {
    if (ctx->ABORT())    return ConflictResolutionMethod::ABORT;
    if (ctx->FAIL())     return ConflictResolutionMethod::FAIL;
    if (ctx->IGNORE())   return ConflictResolutionMethod::IGNORE;
    if (ctx->REPLACE())  return ConflictResolutionMethod::REPLACE;
    if (ctx->ROLLBACK()) return ConflictResolutionMethod::ROLLBACK;

    fail("Unknown resolution method {}", ctx->getText());
}

std::any SqlGrammarVisitor::visitExpr(GrammarParser::ExprContext *ctx) {
    return Expr{ctx->IDENTIFIER()->getText()};
}

std::any SqlGrammarVisitor::visitColumn_alias(GrammarParser::Column_aliasContext *ctx) {
    return ctx->IDENTIFIER()->getText();
}

std::any SqlGrammarVisitor::visitType_name(GrammarParser::Type_nameContext *ctx) {
    return ctx->IDENTIFIER()->getText();
}

std::any SqlGrammarVisitor::visitSchema_name(GrammarParser::Schema_nameContext *ctx) {
    return ctx->IDENTIFIER()->getText();
}

std::any SqlGrammarVisitor::visitTable_alias(GrammarParser::Table_aliasContext *ctx) {
    return ctx->IDENTIFIER()->getText();
}

std::any SqlGrammarVisitor::visitTable_name(GrammarParser::Table_nameContext *ctx) {
    return ctx->IDENTIFIER()->getText();
}

std::any SqlGrammarVisitor::visitColumn_name(GrammarParser::Column_nameContext *ctx) {
    return ctx->IDENTIFIER()->getText();
}


