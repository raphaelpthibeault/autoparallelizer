#include "support/Any.h"
#include "tree/TerminalNode.h"
#include "util.hpp"
#include "visitors.hpp"

VariableVisitor::VariableVisitor(std::set<std::string> &vars_alive, std::set<std::string> &vars_dead)
    : vars_alive(vars_alive), vars_dead(vars_dead) {}

VariableVisitor::VariableVisitor(CParser::StatementContext *ctx, std::set<std::string> &vars_alive, std::set<std::string> &vars_dead)
    : VariableVisitor(vars_alive, vars_dead) {
    visitChildren(ctx);
}

VariableVisitor::VariableVisitor(CParser::ForConditionContext *ctx, std::set<std::string> &vars_alive, std::set<std::string> &vars_dead)
    : VariableVisitor(vars_alive, vars_dead) {
    visitChildren(ctx);
}

VariableVisitor::VariableVisitor(CParser::ExpressionContext *ctx, std::set<std::string> &vars_alive, std::set<std::string> &vars_dead)
    : VariableVisitor(vars_alive, vars_dead) {
    analyze_expression(Function::analyze(get_text(ctx)));
}

VariableVisitor::VariableVisitor(CParser::ForExpressionContext *ctx, std::set<std::string> &vars_alive, std::set<std::string> &vars_dead)
    : VariableVisitor(vars_alive, vars_dead) {
    visitChildren(ctx);
}
VariableVisitor::~VariableVisitor() {}

antlrcpp::Any
VariableVisitor::visitDeclaration(CParser::DeclarationContext *ctx) {
    return visitChildren(ctx);
}

antlrcpp::Any
VariableVisitor::visitAssignmentExpression(CParser::AssignmentExpressionContext *ctx) {
    analyze_expression(Function::analyze(get_text(ctx)));
    return visitChildren(ctx);
}

antlrcpp::Any
VariableVisitor::visitPostfixExpression(CParser::PostfixExpressionContext *ctx) {
    for (auto terminalNode : ctx->Identifier()) {
        std::string id = terminalNode->getText();
        if (ctx->children.size() > 1 && ctx->children[1]->getText() == "=") {
            vars_alive.erase(id);
            vars_dead.insert(id);
        } else {
            vars_alive.insert(id);
            vars_dead.insert(id);
        }
    }
    return visitChildren(ctx);
}

antlrcpp::Any
VariableVisitor::visitExpression(CParser::ExpressionContext *ctx) {
    for (auto assignmentExprCtx : ctx->assignmentExpression()) {
        analyze_expression(Function::analyze(get_text(assignmentExprCtx)));
    }
    return visitChildren(ctx);
}

antlrcpp::Any
VariableVisitor::visitPrimaryExpression(CParser::PrimaryExpressionContext *ctx) {
    antlr4::tree::TerminalNode *id = ctx->Identifier();
    if (id) {
            vars_alive.insert(id->getText());
    }
    return visitChildren(ctx);
}

void
VariableVisitor::analyze_expression(std::pair<std::vector<std::string>, int> pair) {
    std::vector<std::string> var = pair.first;
    int type = pair.second;

    if (type == 0 || type == 2) {
        for (const auto &id : var) {
            vars_alive.insert(id);
        }
    } else if (type == 1) {
        for (const auto &id : var) {
            vars_alive.erase(id);
            vars_dead.insert(id);
        }
    }
}
