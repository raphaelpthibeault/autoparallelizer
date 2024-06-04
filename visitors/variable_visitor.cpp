#include "ParserRuleContext.h"
#include "support/Any.h"
#include "util.hpp"
#include "visitors.hpp"

VariableVisitor::VariableVisitor(std::set<std::string> &vars_alive, std::set<std::string> &vars_dead, std::set<std::string> &vars_found)
    : vars_alive(vars_alive), vars_dead(vars_dead), vars_found(vars_found) {}

VariableVisitor::VariableVisitor(CParser::StatementContext *ctx, std::set<std::string> &vars_alive, std::set<std::string> &vars_dead, std::set<std::string> &vars_found)
    : VariableVisitor(vars_alive, vars_dead, vars_found) {
    add_found_vars(Function::analyze(get_text(ctx)));
    visitChildren(ctx);
}

VariableVisitor::VariableVisitor(CParser::ForConditionContext *ctx, std::set<std::string> &vars_alive, std::set<std::string> &vars_dead, std::set<std::string> &vars_found)
    : VariableVisitor(vars_alive, vars_dead, vars_found) {
    add_found_vars(Function::analyze(get_text(ctx)));
    visitChildren(ctx);
}

VariableVisitor::VariableVisitor(CParser::ExpressionContext *ctx, std::set<std::string> &vars_alive, std::set<std::string> &vars_dead, std::set<std::string> &vars_found)
    : VariableVisitor(vars_alive, vars_dead, vars_found) {
    add_found_vars(Function::analyze(get_text(ctx)));
    visitChildren(ctx);
}

VariableVisitor::VariableVisitor(CParser::ForExpressionContext *ctx, std::set<std::string> &vars_alive, std::set<std::string> &vars_dead, std::set<std::string> &vars_found)
    : VariableVisitor(vars_alive, vars_dead, vars_found) {
    add_found_vars(Function::analyze(get_text(ctx)));
    visitChildren(ctx);
}

VariableVisitor::VariableVisitor(CParser::ForDeclarationContext *ctx, std::set<std::string> &vars_alive, std::set<std::string> &vars_dead, std::set<std::string> &vars_found)
    : VariableVisitor(vars_alive, vars_dead, vars_found) {
    add_found_vars(Function::analyze(get_text(ctx)));
    visitChildren(ctx);
}

VariableVisitor::VariableVisitor(CParser::BlockItemContext *ctx, std::set<std::string> &vars_alive, std::set<std::string> &vars_dead, std::set<std::string> &vars_found)
    : VariableVisitor(vars_alive, vars_dead, vars_found) {
    add_found_vars(Function::analyze(get_text(ctx)));
    visitChildren(ctx);
}

VariableVisitor::~VariableVisitor() {}

antlrcpp::Any
VariableVisitor::visitDeclaration(CParser::DeclarationContext *ctx) {
    // declaration, or declaration and assignment
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
    // ignores the left side of all assignment operators (=, +=, *= ...) and unary operators (++) in one shot
    if (ctx->assignmentExpression() != nullptr) {
        analyze_expression(Function::analyze(get_text(ctx->assignmentExpression())));
    }

    return visitChildren(ctx);
}

antlrcpp::Any
VariableVisitor::visitPostfixExpression(CParser::PostfixExpressionContext *ctx) {
    if (!ctx->Identifier().empty() || !ctx->expression().empty()) {
        analyze_expression(Function::analyze(get_text(ctx)));
    }

/*
    // id->id->id...
    for (auto terminal_node : ctx->Identifier()) {
        std::string id = terminal_node->getText();
        vars_alive.insert(id);
    }

    // id[expr][expr]...
    for (auto expr : ctx->expression()) {
        analyze_expression(Function::analyze(get_text(expr)));
    }
*/

    if (ctx->children.size() >= 3 && ctx->children[1]->getText() == "(") {
        // function call id, lpar, rpar
        analyze_expression(Function::analyze(get_text(ctx)));
    }

    return visitChildren(ctx);
}

antlrcpp::Any
VariableVisitor::visitMultiplicativeExpression(CParser::MultiplicativeExpressionContext *ctx) {
    analyze_expression(ctx);
    return visitChildren(ctx);
}

antlrcpp::Any
VariableVisitor::visitAdditiveExpression(CParser::AdditiveExpressionContext *ctx) {
    analyze_expression(ctx);
    return visitChildren(ctx);
}

antlrcpp::Any
VariableVisitor::visitShiftExpression(CParser::ShiftExpressionContext *ctx) {
    analyze_expression(ctx);
    return visitChildren(ctx);
}

antlrcpp::Any
VariableVisitor::visitRelationalExpression(CParser::RelationalExpressionContext *ctx) {
    analyze_expression(ctx);
    return visitChildren(ctx);
}

antlrcpp::Any
VariableVisitor::visitEqualityExpression(CParser::EqualityExpressionContext *ctx) {
    analyze_expression(ctx);
    return visitChildren(ctx);
}

antlrcpp::Any
VariableVisitor::visitAndExpression(CParser::AndExpressionContext *ctx) {
    analyze_expression(ctx);
    return visitChildren(ctx);
}

antlrcpp::Any
VariableVisitor::visitExclusiveOrExpression(CParser::ExclusiveOrExpressionContext *ctx) {
    analyze_expression(ctx);
    return visitChildren(ctx);
}

antlrcpp::Any
VariableVisitor::visitInclusiveOrExpression(CParser::InclusiveOrExpressionContext *ctx) {
    analyze_expression(ctx);
    return visitChildren(ctx);
}

antlrcpp::Any
VariableVisitor::visitLogicalAndExpression(CParser::LogicalAndExpressionContext *ctx) {
    analyze_expression(ctx);
    return visitChildren(ctx);
}

antlrcpp::Any
VariableVisitor::visitLogicalOrExpression(CParser::LogicalOrExpressionContext *ctx) {
    analyze_expression(ctx);
    return visitChildren(ctx);
}

void
VariableVisitor::analyze_expression(std::vector<std::string> vars) {
    for (const auto &id : vars) {
        vars_alive.insert(id);
    }
}

void
VariableVisitor::analyze_expression(antlr4::ParserRuleContext *ctx) {
    if (ctx->children.size() >= 3) {
        analyze_expression(Function::analyze(get_text(ctx)));
    }
}

void
VariableVisitor::add_found_vars(std::vector<std::string> vars) {
    for (const auto &id : vars) {
        vars_found.insert(id);
    }
}
