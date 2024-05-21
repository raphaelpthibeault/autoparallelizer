#include "util.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

std::string
get_text(antlr4::ParserRuleContext *ctx) {
    size_t start = ctx->start->getStartIndex();
    size_t stop = ctx->stop->getStopIndex();
    antlr4::misc::Interval interval(start, stop);
    return ctx->start->getInputStream()->getText(interval) + "\n";
}

void
print_to_file(const std::string &f_name, const std::string &content) {
    std::ofstream out(f_name);
    if (!out.is_open()) {
        std::ostringstream oss;
        oss << "Error: cannot open file " << f_name << " for writing\n";
        throw std::runtime_error(oss.str());
    }

    out << content;
    out.close();
}
