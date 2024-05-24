#include "concepts.hpp"
#include "util.hpp"
#include <iterator>
#include <utility>
#include <vector>

/* PUBLIC */

Function::Function(const std::string &id, CParser::CompoundStatementContext *body_ctx, antlr4::ParserRuleContext *function_ctx)
    : id(id), body_ctx(body_ctx), function_ctx(function_ctx){}

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


std::string
Function::parallelize(bool reduction_operation) const {
    /* TODO */
    return "";
}

void
Function::parallelize_reduction(StatBlock &block, const std::map<std::string, std::vector<std::string>> &reduction, std::stringstream &parallelized, int tabs) const {
    /* TODO */
}


bool
Function::check_assignment(CParser::AssignmentExpressionContext *assign, std::map<std::string, std::string> &reduction_vars) const {
    if (assign->unaryExpression()->postfixExpression()->primaryExpression()->Identifier() != nullptr) {
            std::string left = assign->unaryExpression()->postfixExpression()->primaryExpression()->Identifier()->getText();

            if (reduction_vars.count(left)) {
                return false;
            }

            if (assign->assignmentOperator()->getText() != "=") {
                return false;
            }

            std::string op = assign->assignmentOperator()->getText();
            std::string code = get_text(assign->assignmentExpression());
            auto analyzed = analyze(code);

            if (analyzed.second != 0) {
                return false;
            }

            const auto &right = analyzed.first;
            int occurrences = 0;
            for (const auto &id : right) {
                if (reduction_vars.count(id)) {
                    reduction_vars.erase(id);
                }
                if (id == left) {
                    ++occurrences;
                }
            }

            if (op == "=") {
                if (occurrences != 1) {
                    return false;
                }
                if (assign->assignmentExpression() != nullptr) {
                    return check_min_max(left, assign->assignmentExpression(), reduction_vars);
                } else {
                    // TODO: support direct assignments
                    return false;
                }
            }

            if (op.find_first_of("+=-=*=/=|=&=^=") != std::string::npos && occurrences != 0) {
                return false;
            }

            reduction_vars[left] = op.substr(0, op.size() - 1);
        }

        return true;}


bool
Function::check_min_max(const std::string &left, CParser::AssignmentExpressionContext *expr2, std::map<std::string, std::string> &reduction_vars) const {
    return true;
}

std::map<std::string, std::vector<std::string>>
Function:: check_reduction(CParser::CompoundStatementContext *ctx) const {
    std::map<std::string, std::string> reduction_vars;

    if (ctx->blockItemList() != nullptr) {
        for (auto *inst : ctx->blockItemList()->blockItem()) {
            if (inst->statement() != nullptr && inst->statement()->expressionStatement() != nullptr) {
                for (auto *assign : inst->statement()->expressionStatement()->expression()->assignmentExpression()) {
                    if (!check_assignment(assign, reduction_vars)) {
                        return {};
                    }
                }
            }
        }
    }

    std::map<std::string, std::vector<std::string>> reduction_ops;
    for (const auto &[id, op] : reduction_vars) {
        reduction_ops[op].push_back(id);
    }

    return reduction_ops;
}

std::pair<std::vector<std::string>, int>
Function::analyze(const std::string& text) {
    std::unordered_set<char> special_chars = {'=', '!', '<', '>', '+', '-', '*', '/', '%', '&', '|', '^', '~', '(', '[', ']', ')', '\'', '"', '{', '}', '#', ',', ' '};
    std::vector<std::string> variables;
    int type = 0;

    std::string current;
    for (char c : text) {
        if (c == '(') {
            current.clear();
        } else if (c == '[') {
            if (!current.empty()) {
                variables.push_back(current);
            }
            current.clear();
        }

        if (special_chars.find(c) == special_chars.end()) {
            if (isdigit(c) && current.empty()) {
                continue;
            }
            current += c;
        } else {
            if (!current.empty()) {
                variables.push_back(current);
                current.clear();
            }
        }
    }

    if (!current.empty()) {
        variables.push_back(current);
    }

    return std::make_pair(variables, type);
}

/* PRIVATE */

void
Function::find_disconnected_components(StatBlock block, std::set<StatBlock> &visited, std::vector<std::pair<StatBlock, int>> &topsort, int curr_cc) {
    visited.insert(block);

    for (const auto &neighbor : dependency_graph[block]) {
        if (!visited.count(neighbor)) {
            find_disconnected_components(neighbor, visited, topsort, curr_cc);
        }
    }

    topsort.emplace_back(block, curr_cc);
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
