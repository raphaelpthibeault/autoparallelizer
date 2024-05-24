#include "concepts.hpp"

Program::Program() : main(nullptr) {}
Program::~Program() {}

void
Program::add(const std::string &new_code) {
    parallelized_code << new_code;
}

void
Program::print_defined_functions() const {
    for (const auto &entry : defined_functions) {
        std::cout << "Function: " << entry.first << " " << entry.second->to_string() << "\n";
    }
}

void
Program::print_call_graph() const {
    for (const auto &entry : call_graph) {
        const auto& function = entry.first;
        const auto& neighbors = entry.second;

        std::cout << function->id << " -> ";
        for (const auto& neighbor : neighbors) {
            std::cout << neighbor->id << " ";
        }
        std::cout << "\n\n";
    }
}

bool
Program::is_defined_function(std::string id) const {
    for (const auto &entry : defined_functions) {
        if (entry.first == id)
            return true;
    }
    return false;
}

bool
Program::is_caller_function(std::string id) const {
    for (const auto &entry : call_graph) {
        if (entry.first->id == id) {
            return true;
        }
    }
    return false;
}
