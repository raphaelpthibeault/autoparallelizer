#include "parallelizer.hpp"
#include <iostream>

void
parallelize(CParser &parser) {
    std::cout << "Not implemented yet.\n";
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
