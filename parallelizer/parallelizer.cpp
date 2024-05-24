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
parallelize(CParser &parser) {
    std::cout << "---------- Parallelization ----------\n";

    Program prog;

    antlr4::tree::ParseTree *foo = parser.compilationUnit();

    //std::cout << "----- visit functions -----\n";
    visit_functions(foo, prog);
    //prog.print_defined_functions();
    //std::cout << "----- !visit functions -----\n";

    //std::cout << "----- visit callgraphs -----\n";
    visit_callgraphs(foo, prog);
    //prog.print_call_graph();
    //std::cout << "----- !visit callgraphs -----\n\n";


    //std::cout << "----- topsort functions -----\n";
    std::vector<std::shared_ptr<Function>> function_order = topsort(prog.call_graph);
    //for (auto func : function_order) {
    //    std::cout << func->id << " " << func->body_ctx->getText() << "\n";
    //}
    //std::cout << "----- !topsort functions -----\n\n";

    /*
    std::cout << "----- build flow graphs -----\n";
    for (auto func : function_order) {
        func->build_flow_graph();
    }
    std::cout << "----- !build flow graphs -----\n\n";
*/


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
