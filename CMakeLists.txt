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

set(SOURCES src/token.cpp
            src/scanner.cpp
            src/parser.cpp
            src/symbol.cpp
            src/diag.cpp
            src/type.cpp
            src/compiler.cpp
            src/util.cpp
            src/ast.cpp
            src/arena.cpp)

add_library(swan ${SOURCES})

add_executable(swanc src/swanc.cpp)
add_executable(tests src/tests.cpp)

target_link_libraries(swanc swan)
target_link_libraries(tests swan)
