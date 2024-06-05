#ifndef __UTIL_HPP
#define __UTIL_HPP

#include <string>
#include <antlr4-runtime.h>
#include <CParser.h>

std::string get_text(const antlr4::ParserRuleContext *);
void print_to_file(const std::string &, const std::string &);
bool is_scope(CParser::BlockItemContext * ctx);
bool is_scope(CParser::StatementContext * ctx);

#endif // __UTIL_HPP
