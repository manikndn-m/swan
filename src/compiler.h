#pragma once

#include "ast.h"
#include "parser.h"
#include "util.h"

namespace swan {

enum ModulePhase {
    mp_parsed,
    mp_symbols_gathered,
    mp_emitted,
};

struct Module {
    ModulePhase phase;
    fs::path filepath;
    Arena ast_arena;
    Block *ast;
};

// The entry point to the compiler.
class Compiler {
    std::vector<fs::path> msearchpaths;

  public:
    Compiler() {}

    void add_searchpath(fs::path p) { msearchpaths.push_back(p); }

    void compile_main(fs::path mainmod_path);
};

} // namespace swan