cmake_minimum_required(VERSION 3.10)
project(swan VERSION 0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(MSVC)
    add_compile_options(/fsanitize=address /Zi /Od /W4)
    add_link_options(/INCREMENTAL:NO)
else()
    # GCC/Clang flags
    add_compile_options(-fsanitize=address -g -O0 -Wall -Wimplicit-fallthrough -D_GLIBCXX_ASSERTIONS)
    add_link_options(-fsanitize=address)
endif()

include_directories(vendor)

set(SOURCES src/token.cc
            src/scanner.cc
            src/parser.cc
            src/symbol.cc
            src/diag.cc
            src/type.cc
            src/compiler.cc
            src/util.cc
            src/ast.cc
            src/arena.cc)

add_library(swan ${SOURCES})

add_executable(swanc src/swanc.cc)
add_executable(tests src/tests.cc)

target_link_libraries(swanc swan)
target_link_libraries(tests swan)
