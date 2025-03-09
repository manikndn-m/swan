cmake_minimum_required(VERSION 3.10)

project(swan VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-fsanitize=address -Wall)
add_link_options(-fsanitize=address)

include_directories(vendor)

set(SOURCES src/token.cpp 
            src/scanner.cpp
            src/parser.cpp
            src/symtbl.cpp
            src/diag.cpp
            src/compiler.cpp
            src/util.cpp
            src/ast.cpp)

add_library(swan ${SOURCES})

add_executable(swanc src/swanc.cpp)
add_executable(tests src/tests.cpp)

target_link_libraries(swanc swan)
target_link_libraries(tests swan)