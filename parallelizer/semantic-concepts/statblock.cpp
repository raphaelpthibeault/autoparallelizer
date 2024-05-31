#include "concepts.hpp"
#include "util.hpp"
#include "visitors.hpp"

StatBlock::StatBlock(int id) : id(id) {}

StatBlock::~StatBlock() {}

void
StatBlock::add_instruction(CParser::BlockItemContext *ctx) {
    instructions.push_back(ctx);
}

void
StatBlock::get_vars_control_struct(CParser::StatementContext *ctx) {
    if (ctx->iterationStatement() != nullptr) {
        get_vars_iters(ctx->iterationStatement());
    } else if (ctx->compoundStatement() != nullptr) {
        if (ctx->compoundStatement()->blockItemList() != nullptr) {
            std::list<CParser::BlockItemContext *> instr_ls;
            for (auto item : ctx->compoundStatement()->blockItemList()->blockItem()) {
                if (item->statement() != nullptr || item->declaration() != nullptr) {
                    instr_ls.push_back(item);
                }
            }
            get_vars(instr_ls);
        }
    } else if (ctx->selectionStatement() != nullptr) {
        if (ctx->selectionStatement()->expression() != nullptr) {
            new VariableVisitor(ctx->selectionStatement()->expression(), vars_alive, vars_dead, vars_found);
        }
        get_vars_control_struct_body(ctx->selectionStatement()->statement(0)->compoundStatement());
        if (ctx->selectionStatement()->Else() != nullptr) {
            get_vars_control_struct_body(ctx->selectionStatement()->statement(1)->compoundStatement());
        }
    }
}

void
StatBlock::get_vars_control_struct_body(CParser::CompoundStatementContext *ctx) {
    if (ctx->blockItemList() != nullptr) {
        std::list<CParser::BlockItemContext *> instr_ls;
        for (auto itm : ctx->blockItemList()->blockItem()) {
            if (itm->statement() != nullptr || itm->declaration() != nullptr) {
                instr_ls.push_back(itm);
            }
        }

        get_vars(instr_ls);
    }
}

void
StatBlock::get_vars_iters(CParser::IterationStatementContext *ctx) {
    if (ctx->For() != nullptr) {
        if (ctx->forCondition()->expression() != nullptr) {
            new VariableVisitor(ctx->forCondition()->expression(), vars_alive, vars_dead, vars_found);
        } else if (ctx->forCondition()->forDeclaration() != nullptr) {
            new VariableVisitor(ctx->forCondition()->forDeclaration(), vars_alive, vars_dead, vars_found);
        }

        get_vars_control_struct_body(ctx->statement()->compoundStatement());

        for (auto expr : ctx->forCondition()->forExpression()) {
            new VariableVisitor(expr, vars_alive, vars_dead, vars_found);
        }
    } else if (ctx->While() != nullptr || ctx->Do() != nullptr) {
        get_vars_control_struct_body(ctx->statement()->compoundStatement());
        if (ctx->expression() != nullptr) {
            new VariableVisitor(ctx->expression(), vars_alive, vars_dead, vars_found);
        }
    }
}

void
StatBlock::get_vars() {
    get_vars(instructions);

    /* populate dead vars */
    for (const auto &var : vars_found) {
        if (vars_alive.find(var) == vars_alive.end()) {
            vars_dead.insert(var);
        }
    }
}

void
StatBlock::get_vars(std::list<CParser::BlockItemContext *> ctxs) {
    if (ctxs.empty()) return;

    for (auto it = ctxs.begin(); it != ctxs.end(); ++it) {
        CParser::BlockItemContext *next = *it;
        if (is_scope2(next)) {
            get_vars_control_struct(next->statement());
        } else {
            new VariableVisitor(next, vars_alive, vars_dead, vars_found);
        }
    }
}

std::string
StatBlock::to_string() const {
    std::stringstream builder;
    builder << "Block #" << id << "\nInstructions:\n";
    builder << "..................\n";
    for (auto inst : instructions) {
        builder << get_text(inst) << "\n";
    }
    builder << "..................\n";
    builder << "Alive variables [ ";
    for (const auto &alive : vars_alive) {
        builder << alive << " ";
    }
    builder << "]\nDead variables [ ";
    for (const auto &dead : vars_dead) {
        builder << dead << " ";
    }
    builder << "]\n";
    builder << "..........................................";
    return builder.str();
}

std::string
StatBlock::get_txt(int tabs) const {
    std::string prefix(tabs, '\t');
    std::stringstream builder;
    for (auto stat : instructions) {
        std::string text = get_text(stat);
        builder << prefix << text << "\n";
    }
    return builder.str();
}
