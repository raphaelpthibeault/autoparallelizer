#ifndef __VISITORS_HPP
#define __VISITORS_HPP

#include <CBaseVisitor.h>
#include <CParser.h>
#include <semantic-concepts/concepts.hpp>
#include <utility>
#include <util.hpp>

class GlobalVisitor : public CBaseVisitor {
public:
    Program &program;

    GlobalVisitor(Program &program);
    ~GlobalVisitor();
    virtual antlrcpp::Any visitTranslationUnit(CParser::TranslationUnitContext *ctx) override;
};


class VariableVisitor : public CBaseVisitor {
private:
    void analyze_expression(std::vector<std::string> var);

public:
    std::set<std::string> &vars_alive, &vars_dead;

    VariableVisitor(std::set<std::string> &vars_alive, std::set<std::string> &vars_dead);
    VariableVisitor(CParser::StatementContext *ctx, std::set<std::string> &vars_alive, std::set<std::string> &vars_dead);
    VariableVisitor(CParser::ForConditionContext *ctx, std::set<std::string> &vars_alive, std::set<std::string> &vars_dead);
    VariableVisitor(CParser::ExpressionContext *ctx, std::set<std::string> &vars_alive, std::set<std::string> &vars_dead);
    VariableVisitor(CParser::ForExpressionContext *ctx, std::set<std::string> &vars_alive, std::set<std::string> &vars_dead);
    VariableVisitor(CParser::ForDeclarationContext *ctx, std::set<std::string> &vars_alive, std::set<std::string> &vars_dead);
    VariableVisitor(CParser::BlockItemContext *ctx, std::set<std::string> &vars_alive, std::set<std::string> &vars_dead);
    ~VariableVisitor();

    virtual antlrcpp::Any visitDeclaration(CParser::DeclarationContext *ctx) override;
    virtual antlrcpp::Any visitAssignmentExpression(CParser::AssignmentExpressionContext *ctx) override;
    virtual antlrcpp::Any visitPostfixExpression(CParser::PostfixExpressionContext *ctx) override;
    virtual antlrcpp::Any visitExpression(CParser::ExpressionContext *ctx) override;
    virtual antlrcpp::Any visitPrimaryExpression(CParser::PrimaryExpressionContext *ctx) override;
};

class FunctionVisitor : public CBaseVisitor {
public:
    Program &program;

    FunctionVisitor(Program &program);
    ~FunctionVisitor();

    virtual antlrcpp::Any visitFunctionDefinition(CParser::FunctionDefinitionContext *ctx) override;

};

class CallGraphVisitor : public CBaseVisitor {
private:
    std::string curr_parent;

public:
    Program &program;


    CallGraphVisitor(Program &program);
    ~CallGraphVisitor();

    virtual antlrcpp::Any visitFunctionDefinition(CParser::FunctionDefinitionContext *ctx) override;
    virtual antlrcpp::Any visitPostfixExpression(CParser::PostfixExpressionContext *ctx) override;
};


#endif // __VISITORS_HPP
