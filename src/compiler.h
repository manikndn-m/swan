#pragma once

#include "ast.h"
#include "parser.h"
#include "util.h"
#include <optional>

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
    std::vector<fs::path> searchpaths; // Module search paths

    struct DepNode {
        std::vector<DepNode *> deps;
    };

    struct DepGraph {
        DepNode *root;
    };

    std::optional<fs::path> searchmod(Path p);
    DepGraph build_from(fs::path mainmod);

  public:
    Compiler()
    {
    }

    void add_searchpath(fs::path p)
    {
        searchpaths.push_back(p);
    }

    void compile_main(fs::path mainmod_path);
};

} // namespace swan