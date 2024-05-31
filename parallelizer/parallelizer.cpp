#include "parallelizer.hpp"
#include "tree/ParseTree.h"
#include <cstdlib>
#include <iostream>
#include <visitors.hpp>
#include <queue>
#include <functional>

void
visit_functions(antlr4::tree::ParseTree *tree, Program &prog) {
    FunctionVisitor f_visitor(prog);
    f_visitor.visit(tree);
}

void
visit_callgraphs(antlr4::tree::ParseTree *tree, Program &prog) {
    CallGraphVisitor cg_visitor(prog);
    cg_visitor.visit(tree);
}

std::vector<std::shared_ptr<Function>>
topsort(std::map<std::shared_ptr<Function>, std::vector<std::shared_ptr<Function>>> &call_graph) {
    std::queue<std::shared_ptr<Function>> q;
    std::unordered_map<std::shared_ptr<Function>, int> degree;
    std::vector<std::shared_ptr<Function>> ordered;

    for (auto &entry : call_graph) {
        degree[entry.first] = 0;
    }

    for (auto &entry : call_graph) {
        for (auto &called : entry.second) {
            degree[called]++;
        }
    }

    for (auto &entry : call_graph) {
        if (degree[entry.first] == 0) {
            q.push(entry.first);
        }
    }

    while (!q.empty()) {
        std::shared_ptr<Function> f = q.front();
        q.pop();

        ordered.push_back(f);

        for (auto &called : call_graph[f]) {
            degree[called]--;
            if (degree[called] == 0) {
                q.push(called);
            }
        }

    }

    return ordered;
}

void
eliminate_dead_code(const std::shared_ptr<Function> &current, std::set<std::shared_ptr<Function>> &visited, std::vector<std::shared_ptr<Function>> &new_order, Program &program) {
    visited.insert(current);
    for (const auto &neigh : program.call_graph[current]) {
        if (visited.find(neigh) == visited.end()) {
            eliminate_dead_code(neigh, visited, new_order, program);
        }
    }
    new_order.push_back(current);
}

std::vector<std::shared_ptr<Function>>
eliminate_dead_code(std::vector<std::shared_ptr<Function>> &order, Program &program) {
    std::set<std::shared_ptr<Function>> visited;
    std::vector<std::shared_ptr<Function>> new_order;

    for (const auto &f : order) {
        if (f->id == "main-0" || f->id == "main-2") {
            eliminate_dead_code(f, visited, new_order, program);
        }
    }

    return new_order;
}



void
parallelize(CParser &parser) {
    std::cout << "---------- Parallelization ----------\n";

    Program prog;

    antlr4::tree::ParseTree *tree = parser.compilationUnit();

    //std::cout << "----- visit functions -----\n";
    visit_functions(tree, prog);
    //prog.print_defined_functions();
    //std::cout << "----- !visit functions -----\n";

    //std::cout << "----- visit callgraphs -----\n";
    visit_callgraphs(tree, prog);
    //prog.print_call_graph();
    //std::cout << "----- !visit callgraphs -----\n\n";

    //std::cout << "----- topsort functions -----\n";
    std::vector<std::shared_ptr<Function>> function_order = topsort(prog.call_graph);
    //for (auto func : function_order) {
    //    std::cout << func->id << " " << func->body_ctx->getText() << "\n";
    //}
    //std::cout << "----- !topsort functions -----\n\n";

    //std::cout << "----- build flow graphs -----\n";
    for (auto func : function_order) {
        func->build_flow_graph();
    }
/*
*/
    //std::cout << "----- !build flow graphs -----\n\n";

    //std::cout << "----- find dependencies -----\n";
    for (auto func : function_order) {
        func->find_dependencies();
    }

    for (auto func : function_order) {
       //func->print_flow_graph();
    }

    //std::cout << "----- !find dependencies -----\n";

    //std::cout << "----- eliminate dead code -----\n";
    function_order = eliminate_dead_code(function_order, prog);
    //std::cout << "----- !eliminate dead code -----\n";

    //std::cout << "----- build dependency graph -----\n";
    for (auto func: function_order) {
        func->build_dependency_graph();
    }

    for (auto func : function_order) {
        //func->print_dependency_graph();
    }
    //std::cout << "----- !build dependency graph -----\n";

    //std::cout << "----- find disconnected components -----\n";
    for (auto func: function_order) {
        //func->find_disconnected_components();
    }
    //std::cout << "----- !find disconnected components -----\n";



    std::cout << "---------- !Parallelization ----------\n";
}

void
parallelize(std::ifstream &file) {
    antlr4::ANTLRInputStream input(file);
    CLexer lexer(&input);

    antlr4::CommonTokenStream tokens(&lexer);
    tokens.fill();

    CParser parser(&tokens);

    parallelize(parser);
}
