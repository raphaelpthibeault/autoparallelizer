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

    if (!(ctx->children.size() >= 3 && ctx->children[1]->getText() == "(")) {
        return visitChildren(ctx);
    }

    /* ctx is a function call at this point

    Notes:
        > caller: parent->second
        > called: ctx
        > we treat functions not defined in the file as an atomic statement
    */

    auto f_called_name = Function::get_virtual_name(ctx);

    // recursive case
    if (f_called_name == parent->first) {
        program.call_graph[parent->second].push_back(parent->second);
    } else if (program.defined_functions.count(f_called_name)) {
        // locally defined function
        program.call_graph[parent->second].push_back(program.defined_functions[f_called_name]);
        if (!program.is_caller_function(f_called_name)) {
            program.call_graph[program.defined_functions[f_called_name]] = std::vector<std::shared_ptr<Function>>();
        }
    }

    return visitChildren(ctx);
}
