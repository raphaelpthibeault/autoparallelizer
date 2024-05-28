#include "support/Any.h"
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

VariableVisitor::VariableVisitor(CParser::ForDeclarationContext *ctx, std::set<std::string> &vars_alive, std::set<std::string> &vars_dead)
    : VariableVisitor(vars_alive, vars_dead) {
        visitChildren(ctx);
    }

VariableVisitor::VariableVisitor(CParser::BlockItemContext *ctx, std::set<std::string> &vars_alive, std::set<std::string> &vars_dead)
    : VariableVisitor(vars_alive, vars_dead) {
        visitChildren(ctx);
    }

VariableVisitor::~VariableVisitor() {}

antlrcpp::Any
VariableVisitor::visitDeclaration(CParser::DeclarationContext *ctx) {
    // a full declaration
    // variables (the ones not being declared) are being observed
    // e.g. int x = y; int x = function(y); int arr[x][y];

    //std::cout << ctx->getText() << "\n";
    return visitChildren(ctx);
}

antlrcpp::Any
VariableVisitor::visitAssignmentExpression(CParser::AssignmentExpressionContext *ctx) {
    // is the rhs
    std::cout << ctx->getText() << "\n";
    //std::cout << ctx->parent->getText() << "\n";

    analyze_expression(Function::analyze(get_text(ctx)));

    return visitChildren(ctx);
}

antlrcpp::Any
VariableVisitor::visitPostfixExpression(CParser::PostfixExpressionContext *ctx) {
    /* visit constructs like id[expr] or id->id */
    /* or func() */

    // is this lhs or rhs?

    for (auto terminal_node : ctx->Identifier()) { // id->id
        std::string id = terminal_node->getText();
        vars_alive.insert(id);
    }

    return visitChildren(ctx);
}

antlrcpp::Any
VariableVisitor::visitExpression(CParser::ExpressionContext *ctx) {
    /* an expression, so obviously all the values are observed and therefore live */
    for (auto assignmentExprCtx : ctx->assignmentExpression()) {
        analyze_expression(Function::analyze(get_text(assignmentExprCtx)));
    }
    return visitChildren(ctx);
}

antlrcpp::Any
VariableVisitor::visitPrimaryExpression(CParser::PrimaryExpressionContext *ctx) {
    // has some of the rhs
    //std::cout << ctx->getText() << "\n";

    antlr4::tree::TerminalNode *id = ctx->Identifier();
    if (id) {
        bool is_func = false;
        if (ctx->parent != nullptr) {
            for (auto child : ctx->parent->children) {
                if (child->getText() == "(" || child->getText() == ")") {
                    is_func = true;
                }
            }
        }

        if (!is_func) {
            vars_alive.insert(id->getText());
        } else {
            // is it an equals or not
            //std::cout << "func: " << id->getText() << " parent context: " << ctx->parent->getText() << "\n";
        }
    }
    return visitChildren(ctx);
}

antlrcpp::Any
VariableVisitor::visitUnaryExpression(CParser::UnaryExpressionContext *ctx) {
    //std::cout << ctx->getText() << "\n";

    return visitChildren(ctx);
}

antlrcpp::Any
VariableVisitor::visitCastExpression(CParser::CastExpressionContext *ctx) {
    // sizeof(var) ???
    //std::cout << ctx->getText() << "\n";

    return visitChildren(ctx);
}


void
VariableVisitor::analyze_expression(std::vector<std::string> var) {
    for (const auto &id : var) {
        vars_alive.insert(id);
    }
}
