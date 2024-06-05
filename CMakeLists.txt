cmake_minimum_required(VERSION 3.29)
project(autoparallelizer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(antlr4-cpp-runtime)

add_executable(${PROJECT_NAME}
    antlr4-gen/CBaseVisitor.cpp
    antlr4-gen/CBaseListener.cpp
    antlr4-gen/CLexer.cpp
    antlr4-gen/CParser.cpp
    antlr4-gen/CVisitor.cpp
    antlr4-gen/CListener.cpp
    parallelizer/naivepar/semantic-concepts/statblock.cpp
    parallelizer/naivepar/semantic-concepts/function.cpp
    parallelizer/naivepar/semantic-concepts/program.cpp
    parallelizer/parallelizer.cpp
    visitors/global_visitor.cpp
    visitors/variable_visitor.cpp
    visitors/function_visitor.cpp
    visitors/callgraph_visitor.cpp
    util/util.cpp
    main.cpp
)

include_directories(
    antlr4-gen/
    parallelizer/
    parallelizer/naivepar/
    parallelizer/taskpar/
    visitors/
    listeners/
    util/
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE antlr4_cpp_runtime
)
