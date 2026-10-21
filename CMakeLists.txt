cmake_minimum_required(VERSION 3.10)
project(swan VERSION 0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(MSVC)
    add_compile_options(
        /W4
        $<$<CONFIG:Debug>:/fsanitize=address>
        $<$<CONFIG:Debug>:/Zi>
        $<$<CONFIG:Debug>:/Od>
        $<$<CONFIG:Release>:/fsanitize=address>
        $<$<CONFIG:Release>:/Zi>
        $<$<CONFIG:Release>:/O2>
    )

    add_link_options(
        $<$<CONFIG:Debug>:/INCREMENTAL:NO>
        $<$<CONFIG:Release>:/INCREMENTAL:NO>
    )

else()
    # GCC/Clang flags
    add_compile_options(
        -Wall
        -Wimplicit-fallthrough
        -D_GLIBCXX_ASSERTIONS

        # Debug
        $<$<CONFIG:Debug>:-fsanitize=address>
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Debug>:-O0>

        # Release
        $<$<CONFIG:Release>:-O2>
        $<$<CONFIG:Release>:-g>
    )

    add_link_options(
        $<$<CONFIG:Debug>:-fsanitize=address>
    )
    
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
