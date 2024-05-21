#include "concepts.hpp"

Program::Program() : main(nullptr) {}
Program::~Program() {}

void
Program::add(const std::string &new_code) {
    parallelized_code << new_code;
}
