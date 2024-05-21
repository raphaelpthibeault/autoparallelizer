cmake_minimum_required(VERSION 3.29)
project(autoparallelizer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(antlr4-cpp-runtime)

add_executable(${PROJECT_NAME}
    antlr4-gen/CBaseVisitor.cpp
    antlr4-gen/CLexer.cpp
    antlr4-gen/CParser.cpp
    antlr4-gen/CVisitor.cpp
    parallelizer/semantic-concepts/statblock.cpp
    parallelizer/semantic-concepts/function.cpp
    parallelizer/semantic-concepts/program.cpp
    parallelizer/parallelizer.cpp
    visitors/global_visitor.cpp
    util/util.cpp
    main.cpp
)

include_directories(
    antlr4-gen/
    parallelizer/
    visitors/
    util/
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE antlr4_cpp_runtime
)
