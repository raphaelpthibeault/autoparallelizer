#include "concepts.hpp"
#include <algorithm>
#include <iterator>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>
#include <queue>

/* PUBLIC */

Function::Function(const std::string &id, CParser::CompoundStatementContext *body_ctx, antlr4::ParserRuleContext *function_ctx)
    : id(id), body_ctx(body_ctx), function_ctx(function_ctx) {}

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

    for (i = 0; i < flow_graph.size(); ++i) {
        for (const auto &alive : flow_graph[i].vars_used) {
            for (j = i - 1; j >= 0; --j) {
                if (flow_graph[j].vars_unused.count(alive)) {
                    dependency_graph[blocks[j]].insert(blocks[i]);
                    break;
                }
            }
        }
        for (const auto &dead : flow_graph[i].vars_unused) {
            for (j = i - 1; j >= 0; --j) {
                if (flow_graph[j].vars_used.count(dead)) {
                    dependency_graph[blocks[j]].insert(blocks[i]);
                }
            }
        }
    }
}

void
Function::print_dependency_graph() {
    for (const auto &[block, neighbors] : dependency_graph) {
        std::cout << id << " " << block.id << " -> ";
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
    if (function_ctx == nullptr)
        return "";

    std::stringstream parallelized;

    if (auto f = dynamic_cast<CParser::FunctionDefinitionContext *>(function_ctx)) {
        if (f->declarationSpecifiers() != nullptr)
            parallelized << get_text(f->declarationSpecifiers());

        parallelized << " " << get_text(f->declarator());

        if (f->declarationList() != nullptr)
            parallelized << get_text(f->declarationList());

    }

    parallelized << " {\n"; // function

    std::vector<std::vector<StatBlock>> sections;

    for (const auto &[block, c] : blocks_components) {
        if (c >= sections.size()) {
            sections.emplace_back();
        }

        sections[c].emplace_back(block);
    }
/*
    for (const auto &section: sections) {
        if (section.size() > 1) {
            parallelized << "\t#pragma omp parallel sections\n\t{\n";
            for (const auto& block : section) {
                parallelized << "\t\t#pragma omp section\n\t\t{\n";
                const auto &instructions = block.instructions;

                if (reduction_operation) {
                    //  if nonempty and is "for" block
                } else {
                   parallelized << block.get_txt(3);
                }
                parallelized << "\t\t}\n";
            }

            parallelized << "\t}\n";
        } else {
            const auto &instructions = section[0].instructions;
            if (reduction_operation) {
                //if nonempty and is "for" block
            } else {
                parallelized << section[0].get_txt(1);
            }
        }
    }*/

    if (sections.size() > 1) {
        // Sections construct
        parallelized << "\t#pragma omp parallel sections\n\t{\n";

        for (const auto &section : sections) {
            parallelized << "\t\t#pragma omp section\n\t\t{\n";

            for (const auto &block : section) {
                parallelized << block.get_txt(3);
            }
            parallelized << "\t\t}\n";
        }
        parallelized << "\t}\n";
    } else if (sections.size() == 1) {
        for (const auto &block : sections[0]) {
            //  if nonempty and is "for" block
            parallelized << block.get_txt(1);
        }
    }

    if (ret_block) {
        parallelized << ret_block->get_txt(1);
    }

    parallelized << "}\n";

    //std::cout << parallelized.str() << "\n\n\n\n";

    return parallelized.str();
}

void
Function::parallelize_reduction(StatBlock &block, const std::map<std::string, std::vector<std::string>> &reduction, std::stringstream &parallelized, int tabs) const {
    /* TODO */
}

bool
Function::check_assignment(CParser::AssignmentExpressionContext *assign, std::map<std::string, std::string> &reduction_vars) const {
    if (assign->unaryExpression()->postfixExpression()->primaryExpression()->Identifier() == nullptr) {
        return true;
    }

    if (assign->assignmentOperator()->getText() != "=") {
        return false;
    }

    std::string left = assign->unaryExpression()->postfixExpression()->primaryExpression()->Identifier()->getText();

    if (reduction_vars.count(left)) {
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

    return true;
}

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

    return variables;
}

std::string
Function::to_string() const {
    std::stringstream ss;
    ss << "ID: " << id << " ;BODY_CTX: " << body_ctx->getText() << "\n";
    return ss.str();
}

void
Function::determine_blocks_components() {
    /* find weekly connected components in a DAG */

    blocks_components.clear();
    std::map<StatBlock, bool> visited;
    int c = 0;

    for (const auto& entry : dependency_graph) {
        if (!visited[entry.first]) {
            explore_component(entry.first, c, visited);
            c++;
        }
    }

    std::sort(blocks_components.begin(), blocks_components.end(), [](const auto &a, const auto &b) {
        if (a.second == b.second) {
            return a.first < b.first;
        }
        return a.second < b.second;
    });
}

void
Function::print_blocks_order() {
    for (const auto &[block, g] : blocks_components) {
        std::cout << id << " Block: " << block.id << ", Component: " << g << "\n";
    }
}

/* PRIVATE */

void
Function::explore_component(const StatBlock& start, int c, std::map<StatBlock, bool>& visited) {
    std::queue<StatBlock> to_explore;
    to_explore.push(start);
    visited[start] = true;

    while (!to_explore.empty()) {
        StatBlock current = to_explore.front();
        to_explore.pop();

        blocks_components.push_back({current, c});

        for (const StatBlock& neighbor : dependency_graph[current]) {
            if (!visited[neighbor]) {
                to_explore.push(neighbor);
                visited[neighbor] = true;
            }
        }

        for (const auto& entry : dependency_graph) {
            if (entry.second.find(current) != entry.second.end() && !visited[entry.first]) {
                to_explore.push(entry.first);
                visited[entry.first] = true;
            }
        }
    }
}

bool
Function::is_scope(CParser::StatementContext *ctx) {
    return ctx->iterationStatement() != nullptr || ctx->compoundStatement() != nullptr || ctx->selectionStatement() != nullptr;
}

void
Function::get_vars() {
    vars_alive.clear();
    vars_dead.clear();
    for (auto it = flow_graph.begin(); it != flow_graph.end(); ++it) {
        //std::cout << "******************************" << id << " BLOCK" << it->id << "******************************\n";
        it->get_vars();
    }
}
