#include "parallelizer.hpp"
#include "semantic-concepts/concepts.hpp"
#include "tree/ParseTree.h"
#include <iostream>
#include <visitors.hpp>

void
visit_functions(CParser &parser, Program &prog) {
    parser.reset();
    antlr4::tree::ParseTree *tree = parser.compilationUnit();

    FunctionVisitor f_visitor(prog);
    f_visitor.visit(tree);
}

void
visit_callgraphs(CParser &parser, Program &prog) {
    parser.reset();
    antlr4::tree::ParseTree *tree = parser.compilationUnit();

    CallGraphVisitor cg_visitor(prog);
    cg_visitor.visit(tree);
}


void
parallelize(CParser &parser) {
    std::cout << "---------- Parallelization ----------\n";

    CParser::TranslationUnitContext *tree = parser.translationUnit();

    Program prog;

    std::cout << "----- visit functions -----\n";
    visit_functions(parser, prog);
    prog.print_defined_functions();
    std::cout << "----- !visit functions -----\n";

    std::cout << "----- visit callgraphs -----\n";
    visit_callgraphs(parser, prog);
    prog.print_call_graph();
    std::cout << "----- !visit callgraphs -----\n";

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
