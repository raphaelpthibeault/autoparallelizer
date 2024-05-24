#include "visitors.hpp"
#include <memory>

FunctionVisitor::FunctionVisitor(Program &program) : program(program) {}

FunctionVisitor::~FunctionVisitor() {}

antlrcpp::Any
FunctionVisitor::visitFunctionDefinition(CParser::FunctionDefinitionContext *ctx) {
    if (ctx->compoundStatement() == nullptr) {
        return nullptr;
    }

    std::string name = Function::get_virtual_name(ctx);
    auto func = std::make_shared<Function>(name, ctx->compoundStatement(), ctx);
    program.defined_functions[name] = func;

    return nullptr;
}
