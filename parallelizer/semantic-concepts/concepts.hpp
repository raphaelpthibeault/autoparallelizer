#ifndef __CONCEPTS_HPP
#define __CONCEPTS_HPP

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
    std::list<CParser::BlockItemContext *> instructions;
    std::set<std::string> vars_used, vars_unused, vars_found;

    int id;

    StatBlock(int id);
    ~StatBlock();

    void add_instruction(CParser::BlockItemContext *ctx);
    void get_vars_control_struct(CParser::StatementContext *ctx);
    void get_vars_control_struct_body(CParser::CompoundStatementContext *ctx);
    void get_vars_iters(CParser::IterationStatementContext *ctx);
    void get_vars();
    void get_vars(std::list<CParser::BlockItemContext *> ctxs);
    std::string to_string() const;
    std::string get_txt(int tabs) const;

    bool operator<(const StatBlock& other) const {
        return id < other.id;
    }

    bool operator==(const StatBlock &other) const {
        return id == other.id;
    }

};


class Function {
public:
    std::vector<StatBlock> flow_graph;
    std::map<StatBlock, std::set<StatBlock>> dependency_graph;
    std::vector<std::pair<StatBlock, int>> blocks_components;
    std::unique_ptr<StatBlock> ret_block;
    std::string id;
    CParser::CompoundStatementContext *body_ctx;
    antlr4::ParserRuleContext *function_ctx;
    std::set<std::string> vars_alive, vars_dead;

    Function(const std::string &id, CParser::CompoundStatementContext *body_ctx, antlr4::ParserRuleContext *function_ctx);
    Function(const std::string &id);
    ~Function();
    void build_flow_graph();
    void print_flow_graph();
    void build_dependency_graph();
    void print_dependency_graph();
    void find_dependencies();
    static std::string get_virtual_name(CParser::FunctionDefinitionContext *ctx);
    static std::string get_virtual_name(CParser::PostfixExpressionContext *ctx);
    std::string parallelize(bool reduction_operation) const;
    void parallelize_reduction(StatBlock &block, const std::map<std::string, std::vector<std::string>> &reduction, std::stringstream &parallelized, int tabs) const;
    bool check_assignment(CParser::AssignmentExpressionContext *assign, std::map<std::string, std::string> &reduction_vars) const;
    bool check_min_max(const std::string &left, CParser::AssignmentExpressionContext *expr2, std::map<std::string, std::string> &reduction_vars) const;
    std::map<std::string, std::vector<std::string>> check_reduction(CParser::CompoundStatementContext *ctx) const;
    static std::vector<std::string> analyze(const std::string& text);
    std::string to_string() const;
    void determine_blocks_components();
    void print_blocks_order();

    bool operator<(const Function& other) const {
        return id < other.id;
    }

    // C doesn't allow function overloads so this is fine
    bool operator==(const Function& other) const {
        return id == other.id;
    }

private:
    bool is_scope(CParser::StatementContext *ctx);
    void get_vars();
    void explore_component(const StatBlock& start, int componentId, std::map<StatBlock, bool>& visited);

};

namespace std {
    template<>
    struct hash<StatBlock> {
        std::size_t operator()(const StatBlock &block) const {
            return std::hash<int>()(block.id);
        }
    };
}

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
    void print_defined_functions();
    void print_call_graph();
    bool is_defined_function(std::string id);
    bool is_caller_function(std::string id);
};


#endif // __CONCEPTS_HPP
