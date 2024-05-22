#include "concepts.hpp"

Function::Function(const std::string &id, CParser::CompoundStatementContext *body_ctx, antlr4::ParserRuleContext *function_ctx)
    : id(id), body_ctx(body_ctx), function_ctx(function_ctx){
    }

Function::Function(const std::string &id) : Function(id, nullptr, nullptr) {}

Function::~Function() {}

void
Function::build_flow_graph() {
    if (body_ctx == nullptr) return;

    int index = 1;

    StatBlock curr(index++);
    for (auto *inst : body_ctx->blockItemList()->blockItem()) {
            if (is_scope(inst->statement())) {
                flow_graph.push_back(curr);
                if (curr.statements.empty()) {
                    curr.add_statement(inst->statement());
                } else {
                    StatBlock controlBlock(index++);
                    controlBlock.add_statement(inst->statement());
                    flow_graph.push_back(controlBlock);
                }
                curr = StatBlock(index++);
            } else if (inst->statement()->jumpStatement() != nullptr && inst->statement()->jumpStatement()->Return() != nullptr) {
                ret_block = std::make_unique<StatBlock>(index++);
                ret_block->add_statement(inst->statement());
            } else if (inst->declaration() != nullptr) {
                if (!decl_block) {
                    decl_block = std::make_unique<StatBlock>(0);
                }
                decl_block->add_statement(inst->statement());
            } else {
                curr.add_statement(inst->statement());
            }
        }

        if (!curr.statements.empty()) {
            flow_graph.push_back(curr);
        }
}
