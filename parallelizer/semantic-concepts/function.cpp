#include "concepts.hpp"
#include <iterator>
#include <vector>

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

void
Function::print_flow_graph() const {
   for (const auto &block : flow_graph) {
       std::cout << block.to_string() << "\n";
   }
}

void
Function::build_dependency_graph() {
    if (flow_graph.empty()) return;
    int i, j;

    std::vector<StatBlock> blocks;
    for (const auto &block : flow_graph) {
        dependency_graph[block] = std::set<StatBlock>();
        blocks.push_back(block);
    }

    for (i = flow_graph.size()-1; i >= 0; --i) {
        for (const auto &alive : flow_graph[i].vars_alive) {
            for (j = i - 1; j >= 0; --j) {
                if (flow_graph[j].vars_dead.count(alive)) {
                    dependency_graph[blocks[j]].insert(blocks[i]);
                    break;
                }
            }
        }
        for (const auto &dead : flow_graph[i].vars_dead) {
            for (j = i - 1; j >= 0; --j) {
                if (flow_graph[j].vars_alive.count(dead)) {
                    dependency_graph[blocks[j]].insert(blocks[i]);
                }
            }
        }
    }
}

void
Function::print_dependency_graph() const {
    for (const auto &[block, neighbors] : dependency_graph) {
        std::cout << block.id << " -> ";
        for (const auto &neighbor : neighbors) {
            std::cout << neighbor.id << " ";
        }
        std::cout << "\n";
    }
}

void
Function::find_dependencies() {
    if (body_ctx == nullptr) return;
    get_vars();
}

void
Function::find_disconnected_components() {
    std::set<StatBlock> visited;
    blocks_order.clear();

    int curr_cc = 0;
    for (const auto &[block, neighbors] : dependency_graph) {
        if (!visited.count(block)) {
            find_disconnected_components(block, visited, blocks_order, curr_cc);
        }
    }

}

std::string
Function::get_virtual_name(CParser::FunctionDefinitionContext *ctx) {
    std::stringstream name;

    name << ctx->declarator()->directDeclarator()->getText();

    if (ctx->declarator()->directDeclarator()->parameterTypeList() != nullptr) {
        name << "-" << ctx->declarator()->directDeclarator()->parameterTypeList()->parameterList()->parameterDeclaration().size();
    }

    return name.str();
}

std::string
Function::get_virtual_name(CParser::PostfixExpressionContext *ctx) {
    std::stringstream name;

    if (!ctx->Identifier().empty()) {
        name << ctx->Identifier(0)->getText();
    }

    if (!ctx->argumentExpressionList().empty()) {
        name << "-" << ctx->argumentExpressionList().size();
    }

    return name.str();
}




void
Function::find_disconnected_components(StatBlock block, std::set<StatBlock> &visited, std::vector<std::pair<StatBlock, int>> &topsort, int curr_cc) {
    visited.insert(block);
    const auto &neighbors = dependency_graph[block];

    for (const auto &neighbor : neighbors) {
        if (!visited.count(neighbor)) {
            find_disconnected_components(neighbor, visited, topsort, curr_cc);
        }
    }

    topsort.emplace(topsort.begin(), curr_cc);
}

bool
Function::is_scope(CParser::StatementContext *ctx) {
    return ctx->iterationStatement() != nullptr || ctx->compoundStatement() != nullptr || ctx->selectionStatement() != nullptr;
}

void
Function::get_vars() {
    vars_alive.clear();
    vars_dead.clear();
    for (auto it = flow_graph.rbegin(); it != flow_graph.rend(); ++it) {
        it->get_vars();
    }
}
