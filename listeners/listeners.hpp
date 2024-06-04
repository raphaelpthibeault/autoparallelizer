#ifndef __LISTENERS_HPP
#define __LISTENERS_HPP

#include <CBaseListener.h>
#include <CParser.h>
#include <semantic-concepts/concepts.hpp>
#include <utility>
#include <util.hpp>
#include <CLexer.h>

class DirectivesAndMacrosListener : public CBaseListener {
public:
    Program &program;

    DirectivesAndMacrosListener(Program &program) : program(program) {}
    ~DirectivesAndMacrosListener() = default;

    void visitTerminal(antlr4::tree::TerminalNode * node) override {
        antlr4::Token *token = node->getSymbol();
        int type = token->getType();

        std::cout << " token " << token->getText() << "\n";

        if (type == CLexer::Directive || type == CLexer::MultiLineMacro) {
            program.add(token->getText() + "\n");
        }
    }
};


#endif // __LISTENERS_HPP
