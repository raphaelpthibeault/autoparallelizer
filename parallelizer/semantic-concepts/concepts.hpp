#ifndef __CONCEPTS_HPP
#define __CONCEPTS_HPP

#include "ParserRuleContext.h"
#include <cstddef>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <sstream>
#include <list>
#include <set>
#include <CParser.h>
#include <util.hpp>


class StatBlock {
public:
    std::list<CParser::StatementContext *> statements;
    std::set<std::string> vars_alive, vars_dead;
    int id;

    StatBlock(int id);
    ~StatBlock();

    void add_statement(CParser::StatementContext *ctx);
    void get_vars_control_struct(CParser::StatementContext *ctx);
    void get_vars_control_struct_body(CParser::CompoundStatementContext *ctx);
    void get_vars_for_loop(CParser::IterationStatementContext *ctx);
    void get_vars();
    void get_vars(std::list<CParser::StatementContext *> ctxs);
    bool is_scope(CParser::StatementContext *ctx) const;
    std::string to_string() const;
    std::string get_txt(int tabs) const;

    bool operator<(const StatBlock& other) const {
        return id < other.id;
    }
};


class Function {
public:
    std::vector<StatBlock> flow_graph;
    std::map<StatBlock, std::set<StatBlock>> dependency_graph;
    std::vector<std::pair<StatBlock, int>> blocks_order;
    std::unique_ptr<StatBlock> decl_block;
    std::unique_ptr<StatBlock> ret_block;
    std::string id;
    CParser::CompoundStatementContext *body_ctx;
    antlr4::ParserRuleContext *function_ctx;
    std::set<std::string> vars_alive, vars_dead;

    Function(const std::string &id, CParser::CompoundStatementContext *body_ctx, antlr4::ParserRuleContext *function_ctx);
    Function(const std::string &id);
    ~Function();
    void build_flow_graph();
    void print_flow_graph() const;
    void build_dependency_graph();
    void print_dependency_graph() const;
    void find_dependencies();
    void find_disconnected_components();
    static std::string get_virtual_name(CParser::FunctionDefinitionContext *ctx);
    static std::string get_virtual_name(CParser::PostfixExpressionContext *ctx);
    std::string parallelize(bool reduction_operation) const;
    void parallelize_reduction(StatBlock &block, const std::map<std::string, std::vector<std::string>> &reduction, std::stringstream &parallelized, int tabs) const;
    bool check_assignment(CParser::AssignmentExpressionContext *assign, std::map<std::string, std::string> &reduction_vars) const;
    bool check_min_max(const std::string &left, CParser::ConditionalExpressionContext *expr2, std::map<std::string, std::string> &reduction_vars) const;
    std::map<std::string, std::vector<std::string>> check_reduction(CParser::CompoundStatementContext *ctx) const;
    static std::pair<std::vector<std::string>, int> analyze(const std::string& text);

    bool operator<(const Function& other) const {
        return id < other.id;
    }

private:
    void find_disconnected_components(StatBlock block, std::set<StatBlock> &visited, std::vector<std::pair<StatBlock, int>> &topsort, int curr_cc);
    bool is_scope(CParser::StatementContext *ctx);
    void get_vars();
};


class Program {
public:
    std::map<std::string, std::shared_ptr<Function>> defined_functions;
    std::map<std::shared_ptr<Function>, std::vector<std::shared_ptr<Function>>> call_graph;
    std::shared_ptr<Function> main;
    std::ostringstream parallelized_code;

    Program();
    ~Program();
    Program(const Program&) = delete;
    Program& operator=(const Program&) = delete;
    void add(const std::string& new_code);
};


#endif // __CONCEPTS_HPP
