#pragma once

#include <fstream>
#include <optional>
#include <sstream>
#include <stack>

#include "ast.h"
#include "parser.h"
#include "symbol.h"
#include "util.h"

namespace swan {

struct CompilerError {
    std::string message;
};

// The entry point to the compiler.
class Compiler {
    fs::path search_path;
    Scope *toplevel_scope, *curr_scope;
    std::stack<Scope*> scope_stack;
    Arena arena;
    std::ofstream cfile;
    std::stringstream c_code;
    int temp_id;
    std::stringstream msg;
    StringRef module_prefix;

    template <typename T>
    void emit(T t) {
        c_code << t;
    }

    template <typename T, typename U, typename... Args>
    void emit(T t, U u, Args... args) {
        c_code << t;
        emit(u, args...);
    }

    template <typename T>
    void err(T t) {
        msg << t;
        ParserError e;
        e.msg = msg.str();
        std::cout << msg.str() << std::endl;
        throw e;
    }

    template <typename T, typename U, typename... Args>
    void err(T t, U u, Args... args) {
        msg << t;
        err(u, args...);
    }

    template <typename T>
    std::string strref_conc(std::string accum, T t) {
        accum = accum + std::string(t);
        return accum;
    }

    template <typename T, typename U, typename... Args>
    std::string strref_conc(std::string accum, T t, U u, Args... args) {
        accum = accum + std::string(t);
        return strref_conc(accum, u, args...);
    }

    template <typename... Args>
    std::string str_from_strrefs(Args... args) {
        return strref_conc("", args...);
    }

    template <typename... Args>
    StringRef alloc_conc_strrefs(Args... args) {
        return arena.alloc_str(str_from_strrefs(args...));
    }

   public:
    Compiler(fs::path search_path) {
        this->search_path = search_path;
        this->toplevel_scope = new Scope(nullptr);
        this->curr_scope = this->toplevel_scope;
        this->scope_stack.push(this->curr_scope);
        this->temp_id = 0;
    }

    ~Compiler() {
        while (!this->scope_stack.empty()) {
            delete this->scope_stack.top();
            this->scope_stack.pop();
        }
    }

    void compile(fs::path filepath, bool is_main_mod);

   private:
    fs::path find_module_file(Path p);
    Symbol resolve_path(Path p);
    Type* type_ann_to_type(TypeAnnot* typeann, Scope* scope);
    void add_symbol_from_stmt(AstNode* stmt, Scope* scope);

    Type* infer_type(AstNode* expr, Scope* scope);
    void emit_expr(AstNode* expr, Scope* scope);
    StringRef type_to_ctype(Type* type, Scope* scope);

    struct Tmp {
        int id;
    };

    void emit_block(Block* block, bool surround_with_braces);
    void emit_stmt(AstNode* stmt);
};

}  // namespace swan