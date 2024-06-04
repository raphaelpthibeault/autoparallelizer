#ifndef __VISITORS_HPP
#define __VISITORS_HPP

#include <CBaseVisitor.h>
#include <CParser.h>
#include <semantic-concepts/concepts.hpp>
#include <utility>
#include <util.hpp>
#include <CLexer.h>

class GlobalVisitor : public CBaseVisitor {
public:
    Program &program;

    GlobalVisitor(Program &program);
    ~GlobalVisitor();
    virtual antlrcpp::Any visitTranslationUnit(CParser::TranslationUnitContext *ctx) override;
};


class VariableVisitor : public CBaseVisitor {
private:
    void analyze_expression(std::vector<std::string> vars);
    void analyze_expression(antlr4::ParserRuleContext *ctx);
    void add_found_vars(std::vector<std::string> vars);

public:
    std::set<std::string> &vars_alive, &vars_dead, &vars_found;

    VariableVisitor(std::set<std::string> &vars_alive, std::set<std::string> &vars_dead, std::set<std::string> &vars_found);
    VariableVisitor(CParser::StatementContext *ctx, std::set<std::string> &vars_alive, std::set<std::string> &vars_dead, std::set<std::string> &vars_found);
    VariableVisitor(CParser::ForConditionContext *ctx, std::set<std::string> &vars_alive, std::set<std::string> &vars_dead, std::set<std::string> &vars_found);
    VariableVisitor(CParser::ExpressionContext *ctx, std::set<std::string> &vars_alive, std::set<std::string> &vars_dead, std::set<std::string> &vars_found);
    VariableVisitor(CParser::ForExpressionContext *ctx, std::set<std::string> &vars_alive, std::set<std::string> &vars_dead, std::set<std::string> &vars_found);
    VariableVisitor(CParser::ForDeclarationContext *ctx, std::set<std::string> &vars_alive, std::set<std::string> &vars_dead, std::set<std::string> &vars_found);
    VariableVisitor(CParser::BlockItemContext *ctx, std::set<std::string> &vars_alive, std::set<std::string> &vars_dead, std::set<std::string> &vars_found);
    ~VariableVisitor();

    virtual antlrcpp::Any visitDeclaration(CParser::DeclarationContext *ctx) override;
    virtual antlrcpp::Any visitAssignmentExpression(CParser::AssignmentExpressionContext *ctx) override;
    virtual antlrcpp::Any visitPostfixExpression(CParser::PostfixExpressionContext *ctx) override;

    virtual antlrcpp::Any visitMultiplicativeExpression(CParser::MultiplicativeExpressionContext *ctx) override;
    virtual antlrcpp::Any visitAdditiveExpression(CParser::AdditiveExpressionContext *ctx) override;
    virtual antlrcpp::Any visitShiftExpression(CParser::ShiftExpressionContext *ctx) override;
    virtual antlrcpp::Any visitRelationalExpression(CParser::RelationalExpressionContext *ctx) override;
    virtual antlrcpp::Any visitEqualityExpression(CParser::EqualityExpressionContext *ctx) override;
    virtual antlrcpp::Any visitAndExpression(CParser::AndExpressionContext *ctx) override;
    virtual antlrcpp::Any visitExclusiveOrExpression(CParser::ExclusiveOrExpressionContext *ctx) override;
    virtual antlrcpp::Any visitInclusiveOrExpression(CParser::InclusiveOrExpressionContext *ctx) override;
    virtual antlrcpp::Any visitLogicalAndExpression(CParser::LogicalAndExpressionContext *ctx) override;
    virtual antlrcpp::Any visitLogicalOrExpression(CParser::LogicalOrExpressionContext *ctx) override;

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

/*
class DirectivesAndMacrosListener : public antlr4::tree::ParseTreeListener {
public:
    Program &program;

    DirectivesAndMacrosListener(Program &program) : program(program) {}
    ~DirectivesAndMacrosListener() = default;

    void visitTerminal(antlr4::tree::TerminalNode * node) override {
        antlr4::Token *token = node->getSymbol();
        int type = token->getType();

        if (type == CLexer::Directive || type == CLexer::MultiLineMacro) {
            program.add(token->getText() + "\n");
        }
    }
};*/



#endif // __VISITORS_HPP
