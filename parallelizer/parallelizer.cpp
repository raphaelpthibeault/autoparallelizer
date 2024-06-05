#include "parallelizer.hpp"
#include <listeners.hpp>
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
sort(std::map<std::shared_ptr<Function>, std::vector<std::shared_ptr<Function>>> &call_graph) {
    std::unordered_map<std::shared_ptr<Function>, bool> visited;
    std::vector<std::shared_ptr<Function>> ordered;
    std::stack<std::shared_ptr<Function>> stack;

    auto visit = [&](std::shared_ptr<Function> f, auto&& visit) -> void {
        if (visited[f]) {
            return;
        }

        visited[f] = true;

        for (auto &called : call_graph[f]) {
            if (!visited[called]) {
                visit(called, visit);
            }
        }

        ordered.push_back(f);
    };

    for (auto &entry : call_graph) {
        if (!visited[entry.first]) {
            visit(entry.first, visit);
        }
    }

    std::reverse(ordered.begin(), ordered.end());
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
        if (f->id == "main-0" || f->id == "main-1" || f->id == "main-2") {
            eliminate_dead_code(f, visited, new_order, program);
        }
    }

    return new_order;
}

void
parallelize(CParser &parser, std::string &directives_and_macros) {
    //std::cout << "---------- Parallelization ----------\n";
    Program prog;
    prog.add(directives_and_macros);
    prog.add("#include <omp.h>\n\n");
    antlr4::tree::ParseTree *tree = parser.compilationUnit();

    GlobalVisitor gv(prog);
    gv.visit(tree);

    //std::cout << "----- visit functions -----\n";
    visit_functions(tree, prog);
    //prog.print_defined_functions();
    //std::cout << "----- !visit functions -----\n";

    //std::cout << "----- visit callgraphs -----\n";
    visit_callgraphs(tree, prog);
    //prog.print_call_graph();

    //std::cout << "----- !visit callgraphs -----\n\n";

    //std::cout << "----- topsort functions -----\n";
    std::vector<std::shared_ptr<Function>> function_order = sort(prog.call_graph);
    //for (auto func : function_order) {
    //    std::cout << func->id << " " << "\n";
    //}
    //std::cout << "----- !topsort functions -----\n\n";

    //std::cout << "----- build flow graphs -----\n";
    for (auto func : function_order) {
        func->build_flow_graph();
    }
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

    /*
    for (auto func : function_order) {
        //func->print_dependency_graph();
    }
    std::cout << "\n\n";
*/
    //std::cout << "----- !build dependency graph -----\n";

    //std::cout << "----- find disconnected components -----\n";


    for (auto func: function_order) {
        //func->find_disconnected_components();
        func->determine_blocks_components();
    }

    for (auto func : function_order) {
       // func->print_blocks_order();
    }

    //std::cout << "----- !find disconnected components -----\n";


    for (auto func: function_order) {
        prog.add(func->parallelize(false));
    }

    // write to file

    std::string filename = "parallelized.c";

    std::ofstream outfile(filename);
    if (!outfile) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    outfile << prog.parallelized_code.str();
    outfile.close();

    //std::cout << "---------- !Parallelization ----------\n";
}

void
parallelize(std::ifstream &file) {
    antlr4::ANTLRInputStream input(file);
    CLexer lexer(&input);

    antlr4::CommonTokenStream tokens(&lexer);
    tokens.fill();


    std::string directives_and_macros;
    for (antlr4::Token *t : tokens.getTokens()) {
        int type = t->getType();
        std::string text = t->getText();

        if (type == CLexer::Directive || type == CLexer::MultiLineMacro) {
            directives_and_macros += text + "\n";
        }
    }

    CParser parser(&tokens);

    parallelize(parser, directives_and_macros);
}
