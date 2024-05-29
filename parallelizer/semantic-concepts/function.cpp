#include "CParser.h"
#include "concepts.hpp"
#include "util.hpp"
#include <iterator>
#include <sstream>
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
        if (is_scope2(inst)) {
            if (curr.instructions.empty()) {
                curr.add_instruction(inst);
                flow_graph.push_back(curr);
            } else {
                flow_graph.push_back(curr);
                StatBlock ctrl_block(index++);
                ctrl_block.add_instruction(inst);
                flow_graph.push_back(ctrl_block);
            }

            curr = StatBlock(index++);
        } else if (inst->statement() != nullptr && inst->statement()->jumpStatement() != nullptr && inst->statement()->jumpStatement()->Return() != nullptr) {
            ret_block = std::make_unique<StatBlock>(index++);
            ret_block->add_instruction(inst);
        } else if (inst->declaration() != nullptr) {
            /* declarations (and declaration + assignment in one step) */
            /* what if I keep the decl_block just for the declarations e.g. 'int x;' */
            curr.add_instruction(inst);
        } else {
            /* function calls and assignments */
            curr.add_instruction(inst);
        }
    }

    if (!curr.instructions.empty()) {
        flow_graph.push_back(curr);
    }
}

void
Function::print_flow_graph() {
    for (const auto &block : flow_graph) {
       std::cout << id << " " << block.to_string() << "\n";
   }
   std::cout << "\n";
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
Function::print_dependency_graph() {
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

    name << ctx->declarator()->directDeclarator()->directDeclarator()->getText();

    if (ctx->declarator()->directDeclarator()->parameterTypeList() != nullptr) {
        name << "-" << ctx->declarator()->directDeclarator()->parameterTypeList()->parameterList()->parameterDeclaration().size();
    } else {
        name << "-0";
    }

    return name.str();
}

std::string
Function::get_virtual_name(CParser::PostfixExpressionContext *ctx) {
    std::stringstream name;

    if (ctx->primaryExpression()->Identifier() != nullptr) {
        name << ctx->primaryExpression()->Identifier()->getText();
    }
    name << "-" << ctx->argumentExpressionList().size();

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

            int occurrences = 0;
            for (const auto &id : analyzed) {
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
    /* TODO */
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

const std::unordered_set<std::string> c_keywords = {
    "auto", "break", "case", "char", "const", "continue", "default", "do", "double",
    "else", "enum", "extern", "float", "for", "goto", "if", "int", "long", "register",
    "return", "short", "signed", "sizeof", "static", "struct", "switch", "typedef",
    "union", "unsigned", "void", "volatile", "while", "_Bool", "_Complex", "_Imaginary"
};

const std::unordered_set<char> special_chars = {'=', '!', '<', '>', '+', '-', '*', '/', '%', '&', '|', '^', '~', '(', '[', ']', ')', '\'', '"', '{', '}', '#', ',', ';', ' '};

std::vector<std::string>
Function::analyze(const std::string& text) {
    std::vector<std::string> variables;
    bool skip = false;

   // std::cout << "ANALYZING TEXT: " << text;

    std::string current;
    for (char c : text) {
        if (c == '"')
            skip = !skip;

        if (skip)
            continue;

        if (c == '(') {
            current.clear();
        } else if (c == '[') {
             if (!current.empty()) {
                variables.push_back(current);
            }
            current.clear();
        } else if (special_chars.find(c) == special_chars.end()) {
            if (isdigit(c) && current.empty()) {
                continue;
            }
            current += c;
        } else if (!current.empty() && c_keywords.find(current) == c_keywords.end()) {
            variables.push_back(current);
            current.clear();
        } else if (!current.empty()) {
            current.clear();
        }
    }

    if (!current.empty() && current != "\n") {
        if (current.back() == '\n') {
            current.pop_back();
        }

        variables.push_back(current);
    }
/*
    std::cout << "GAVE THESE VARIABLES: ";
    for (auto var : variables) {
        std::cout << var << " ";
    }
    std::cout << "\n";
*/
    return variables;
}

std::string
Function::to_string() const {
    std::stringstream ss;
    ss << "ID: " << id << " ;BODY_CTX: " << body_ctx->getText() << "\n";
    return ss.str();
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
