#ifndef __UTIL_HPP
#define __UTIL_HPP

#include <string>
#include <antlr4-runtime.h>

std::string get_text(antlr4::ParserRuleContext *);
void print_to_file(const std::string &, const std::string &);

#endif // __UTIL_HPP
