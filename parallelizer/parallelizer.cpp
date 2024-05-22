#include "parallelizer.hpp"
#include "semantic-concepts/concepts.hpp"
#include <iostream>
#include <visitors.hpp>

void
parallelize(CParser &parser) {
    std::cout << "---------- Begin Parallelization ----------\n";
    //std::cout << "Not implemented yet.\n";

    CParser::TranslationUnitContext *tree = parser.translationUnit();

    Program prog;
    GlobalVisitor visitor(prog);
    visitor.visitTranslationUnit(tree);

    std::cout << "'" << prog.parallelized_code.str() << "'\n";

    std::cout << "---------- End Parallelization ----------\n";
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
