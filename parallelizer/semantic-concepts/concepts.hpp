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


class StatBlock {
private:
    std::list<std::shared_ptr<CParser::StatementContext>> statements;
    std::set<std::string> vars_alive, vars_dead;
    int id;

public:
    StatBlock(int id);
    ~StatBlock();

    void add_statement(std::shared_ptr<CParser::StatementContext> stat);
    const std::list<std::shared_ptr<CParser::StatementContext>>& get_statements() const;
    void get_vars_control_struct(std::shared_ptr<CParser::StatementContext> stat);
    void get_vars_control_struct_body(std::shared_ptr<CParser::CompoundStatementContext> body);
    void get_vars_for_loop(std::shared_ptr<CParser::IterationStatementContext> body);
    void get_vars();
    void get_vars(std::list<std::shared_ptr<CParser::StatementContext>> stats);
    bool is_scope(std::shared_ptr<CParser::StatementContext> stat) const;
    std::string get_text(int tabs) const;
    std::string to_string() const;

    bool operator<(const StatBlock& other) const {
        return id < other.id;
    }
};


class Function : public std::enable_shared_from_this<Function> {
private:
    std::list<StatBlock> flow_graph;
    std::map<StatBlock, std::set<StatBlock>> dependency_graph;
    std::list<std::pair<StatBlock, int>> blocks_order;
    StatBlock decl_block;
    StatBlock ret_block;
    std::string id;
    std::shared_ptr<CParser> body_ctx;
    std::shared_ptr<CParser> func_ctx;
    std::set<std::string> vars_alive, vars_dead;

public:
    Function(const std::string &id, std::shared_ptr<CParser> body_ctx = nullptr, std::shared_ptr<CParser> func_ctx = nullptr);
    ~Function();
    void build_flow_graph();
    void print_flow_graph() const;
    void build_dependency_graph();
    void print_dependency_graph() const;
    void find_dependencies();
    void find_islands();
    static std::string get_virtual_name(std::shared_ptr<CParser> ctx);

    bool operator<(const Function& other) const {
        return id < other.id;
    }

    private:
    bool is_scope(std::shared_ptr<CParser> inst) const;
    void find_islands(StatBlock block, std::set<StatBlock> &visited, std::list<std::pair<StatBlock, int>>& topsort, int current_island) const;
    void parallelize_reduction(StatBlock block, const std::map<std::string, std::vector<std::string>>& reduction, std::ostringstream& parallelized, int tabs) const;
    bool check_assignment(std::shared_ptr<CParser> assign, std::map<std::string, std::string>& reduction_vars) const;
    bool check_min_max(const std::string& left, std::shared_ptr<CParser> expr2, std::map<std::string, std::string>& reduction_vars) const;
    std::map<std::string, std::vector<std::string>> check_reduction(std::shared_ptr<CParser> ctx) const;
    std::pair<std::vector<std::string>, int> analyze(const std::string& text) const;
    std::string get_text(std::shared_ptr<CParser> ctx) const;
};


class Program {
private:
    std::map<std::string, std::shared_ptr<Function>> defined_functions;
    std::map<std::shared_ptr<Function>, std::vector<std::shared_ptr<Function>>> call_graph;
    std::shared_ptr<Function> main;
    std::ostringstream parallelized_code;
public:
    Program();
    ~Program();
    void add(const std::string& new_code);
};


#endif // __CONCEPTS_HPP