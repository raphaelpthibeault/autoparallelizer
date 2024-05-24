#include "visitors.hpp"
#include <memory>

CallGraphVisitor::CallGraphVisitor(Program &prog) : program(prog) {}

CallGraphVisitor::~CallGraphVisitor() {}

antlrcpp::Any
CallGraphVisitor::visitFunctionDefinition(CParser::FunctionDefinitionContext *ctx) {
    curr_parent = Function::get_virtual_name(ctx);
    return visitChildren(ctx);
}

antlrcpp::Any
CallGraphVisitor::visitPostfixExpression(CParser::PostfixExpressionContext *ctx) {
    auto parent = program.defined_functions.find(curr_parent);

    if (parent == program.defined_functions.end()) {
        return visitChildren(ctx);
    }

    if (ctx->primaryExpression()->Identifier() == nullptr) {
        return visitChildren(ctx);
    }

    /* ctx is a function call at this point

    Notes:
        > caller: parent->second
        > callee: ctx
        > we treat functions not defined in the file as an atomic statement
    */

    auto f_called = std::make_shared<Function>(Function::get_virtual_name(ctx));

    if (program.defined_functions.count(f_called->id)) {

        program.call_graph[parent->second].push_back(f_called);

        //if (program.call_graph.find(f_called) == program.call_graph.end()) {
        if (!program.is_caller_function(f_called->id)) {
            program.call_graph[f_called] = std::vector<std::shared_ptr<Function>>();
        } else {
        }

    }

    return visitChildren(ctx);
}
