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
    // declaration, or declaration and assignment
    // variables (the ones not being declared) are being observed
    // e.g. int x = y; int x = function(y); int arr[x][y];

    if (ctx->initDeclaratorList() != nullptr) {
        for (auto init_decl : ctx->initDeclaratorList()->initDeclarator()) {
            if (init_decl->Assign() != nullptr && init_decl->initializer() != nullptr) {
                analyze_expression(Function::analyze(get_text(init_decl->initializer())));
            } else {
                // declared but not assigned

                if (init_decl->declarator()->directDeclarator()->Identifier() == nullptr) {
                    for (auto dd = init_decl->declarator()->directDeclarator(); dd != nullptr && dd->assignmentExpression()!=nullptr; dd = dd->directDeclarator()) {

                        analyze_expression(Function::analyze(get_text(dd->assignmentExpression())));

                    }
                }
            }
        }
    }

    return visitChildren(ctx);
}

antlrcpp::Any
VariableVisitor::visitAssignmentExpression(CParser::AssignmentExpressionContext *ctx) {
    //std::cout << get_text(ctx) << "\n";

    analyze_expression(Function::analyze(get_text(ctx)));
    return visitChildren(ctx);
}

antlrcpp::Any
VariableVisitor::visitPostfixExpression(CParser::PostfixExpressionContext *ctx) {
    // id->id->id...
    for (auto terminal_node : ctx->Identifier()) {
        std::string id = terminal_node->getText();
        vars_alive.insert(id);
    }

    // id[expr][expr]...
    for (auto expr : ctx->expression()) {
        analyze_expression(Function::analyze(get_text(expr)));
    }

    return visitChildren(ctx);
}

antlrcpp::Any
VariableVisitor::visitExpression(CParser::ExpressionContext *ctx) {
    /* an expression, so obviously all the values are observed and therefore live */

   //std::cout << get_text(ctx) << "\n";

    for (auto assignmentExprCtx : ctx->assignmentExpression()) {
        analyze_expression(Function::analyze(get_text(assignmentExprCtx)));
    }
    return visitChildren(ctx);
}

antlrcpp::Any
VariableVisitor::visitPrimaryExpression(CParser::PrimaryExpressionContext *ctx) {
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
            std::cout << get_text(ctx) << "\n";
            // is it an equals or not

        }
    }
    return visitChildren(ctx);
}



void
VariableVisitor::analyze_expression(std::vector<std::string> var) {
    for (const auto &id : var) {
        vars_alive.insert(id);
    }
}
