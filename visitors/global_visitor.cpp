#include "visitors.hpp"
#include <util.hpp>

GlobalVisitor::GlobalVisitor(Program &program) : program(program) {}

GlobalVisitor::~GlobalVisitor() {}

antlrcpp::Any GlobalVisitor::visitTranslationUnit(CParser::TranslationUnitContext *ctx) {
    for (auto global : ctx->externalDeclaration()) {
        if (global->functionDefinition() == nullptr) {
            program.add(get_text(global));
        }
    }

    return nullptr;
}
