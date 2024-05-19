#include <iostream>

#include <antlr4-runtime.h>
#include <CLexer.h>
#include <CParser.h>

int
main() {

    antlr4::ANTLRInputStream input(u8"🍴 = 🍐 + \"😎\";(((x * π))) * µ + ∰; a + (x * (y ? 0 : 1) + z);");

    CLexer lexer(&input);

    antlr4::CommonTokenStream tokens(&lexer);

    tokens.fill();
    for (auto token : tokens.getTokens()) {
        std::cout << token->toString() << std::endl;
    }

    CParser parser(&tokens);

    antlr4::tree::ParseTree *tree = parser.compilationUnit();

    std::cout << tree->toStringTree(&parser) << std::endl << std::endl;

    return 0;
}
