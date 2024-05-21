#ifndef __PARALLELIZER_HPP
#define __PARALLELIZER_HPP

#include <fstream>
#include <semantic-concepts/concepts.hpp>
#include <antlr4-runtime.h>
#include <CLexer.h>
#include <CParser.h>

void parallelize(std::ifstream&);

#endif // __PARALLELIZER_HPP
