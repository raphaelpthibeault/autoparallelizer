#include "concepts.hpp"
#include "util.hpp"
#include "visitors.hpp"

StatBlock::StatBlock(int id) : id(id) {}

StatBlock::~StatBlock() {}

void
StatBlock::add_statement(CParser::StatementContext *ctx) {
    statements.push_back(ctx);
}

void
StatBlock::get_vars_control_struct(CParser::StatementContext *ctx) {
    if (ctx->iterationStatement() != nullptr) {
        get_vars_for_loop(ctx->iterationStatement());
    } else if (ctx->compoundStatement() != nullptr) {
        get_vars(std::list<CParser::StatementContext *>(ctx->compoundStatement()->blockItemList()->blockItem().begin(), ctx->compoundStatement()->blockItemList()->blockItem().end()));
    } else if (ctx->selectionStatement() != nullptr) {
        if (ctx->selectionStatement()->expression() != nullptr) {
            new VariableVisitor(ctx->selectionStatement()->expression(), vars_alive, vars_dead);
        }
        get_vars_control_struct_body(ctx->selectionStatement()->statement(0)->compoundStatement());
        if (ctx->selectionStatement()->Else() != nullptr) {
            get_vars_control_struct_body(ctx->selectionStatement()->statement(1)->compoundStatement());
        }
    }
}

void
StatBlock::get_vars_control_struct_body(CParser::CompoundStatementContext *ctx) {
    if (ctx->blockItemList() != nullptr) {
        get_vars(std::list<CParser::StatementContext *>(ctx->blockItemList()->blockItem().begin(), ctx->blockItemList()->blockItem().end()));
    }
}

void
StatBlock::get_vars_for_loop(CParser::IterationStatementContext *ctx) {
    if (ctx->For() != nullptr) {
        if (ctx->forCondition()->expression() != nullptr) {
            new VariableVisitor(ctx->forCondition()->expression(), vars_alive, vars_dead);
        }

        get_vars_control_struct_body(ctx->statement()->compoundStatement());

        for (auto expr : ctx->forCondition()->forExpression()) {
            new VariableVisitor(expr, vars_alive, vars_dead);
        }
    } else if (ctx->While() != nullptr || ctx->Do() != nullptr) {
        get_vars_control_struct_body(ctx->statement()->compoundStatement());
        if (ctx->expression() != nullptr) {
            new VariableVisitor(ctx->expression(), vars_alive, vars_dead);
        }
    }
}

void
StatBlock::get_vars() {
    vars_alive.clear();
    vars_dead.clear();
    get_vars(statements);
}

void
StatBlock::get_vars(std::list<CParser::StatementContext *> ctxs) {
    if (ctxs.empty()) return;

    for (auto it = statements.rbegin(); it != ctxs.rend(); ++it) {
        CParser::StatementContext *next = *it;
        if (is_scope(next)) {
            get_vars_control_struct(next);
        } else if (next->expressionStatement() != nullptr && next->expressionStatement()->expression() != nullptr) {
            new VariableVisitor(next->expressionStatement()->expression(), vars_alive, vars_dead);
        } else {
            new VariableVisitor(next, vars_alive, vars_dead);
        }
    }
}

bool
StatBlock::is_scope(CParser::StatementContext *ctx) const {
    return ctx->iterationStatement() != nullptr || ctx->compoundStatement() != nullptr
        || ctx->selectionStatement() != nullptr;
}

std::string
StatBlock::to_string() const {
    std::stringstream builder;
    builder << "Block #" << id << "\nInstructions:\n";
    builder << "..........................................\n";
    for (auto stat : statements) {
        builder << get_text(stat) << "\n\n";
    }
    builder << "..........................................\n";
    builder << "Alive variables [ ";
    for (const auto &alive : vars_alive) {
        builder << alive << " ";
    }
    builder << "]\nDead variables [ ";
    for (const auto &dead : vars_dead) {
        builder << dead << " ";
    }
    builder << "]\n";
    builder << "..........................................";
    return builder.str();
}

std::string
StatBlock::get_txt(int tabs) const {
    std::string prefix(tabs, '\t');
    std::stringstream builder;
    for (auto stat : statements) {
        std::string text = get_text(stat);
        builder << prefix << text << "\n";
    }
    return builder.str();
}
