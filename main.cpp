#include <parallelizer.hpp>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <antlr4-runtime.h>
#include <CLexer.h>
#include <CParser.h>

void
sanity_check(std::ifstream &file) {
    antlr4::ANTLRInputStream input(file);

    CLexer lexer(&input);

    antlr4::CommonTokenStream tokens(&lexer);

    tokens.fill();
    for (auto token : tokens.getTokens()) {
        std::cout << token->toString() << std::endl;
    }

    CParser parser(&tokens);

    antlr4::tree::ParseTree *tree = parser.compilationUnit();

    std::cout << tree->toStringTree(&parser) << std::endl << std::endl;
}

bool
is_c_file(const std::string& f) {
    return f.length() >= 2 && f.substr(f.size() - 2) == ".c";
}

int
main(int argc, char** argv) {
    if (argc < 2 || !is_c_file(argv[1])) {
        std::cerr << "Usage: " << argv[0] << " <c file>\n";
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Error: Could not open file " << argv[1] << "\n";
        return 1;
    }

    parallelize(file);

    file.close();
    return 0;
}
