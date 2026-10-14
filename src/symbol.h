#pragma once

#include "type.h"
#include <cassert>
#include <optional>
#include <unordered_map>
#include <variant>

namespace swan {

class Scope;

struct Field {
    Type *type;
    StringRef name;
};

struct Struct {
    StringRef canon_name;
    bool is_object;
    std::unordered_map<StringRef, Field> fields;

    Type *struct_type;
};

struct Param {
    StringRef name;
    Type *type;
};

struct Var {
    Type *type;
    StringRef canon_name;
};

struct FunctionSignature {
    std::vector<Param> params;
    Type *return_type;
};

struct FunctionProto {
    bool has_receiver;
    Type *receiver;
    StringRef func_name;
    FunctionSignature sign;
};

struct Function {
    StringRef canon_name;
    FunctionProto proto;
};

struct ImportedModule {
    Scope *scope;
    Arena ast_arena;
    Block *block;
};

using Symbol = std::variant<Struct *, Function *, Var *>;

class Scope {
    Scope *outer;
    std::unordered_map<StringRef, Symbol> symbols;

  public:
    Scope(Scope *outer) {
        this->outer = outer;
    }

    void add(StringRef name, Symbol s) {
        symbols[name] = s;
    }

    std::optional<Symbol> find(StringRef name) {
        auto it = symbols.find(name);
        if (it == symbols.end()) {
            if (outer) {
                return outer->find(name);
            } else {
                return std::nullopt;
            }
        }

        return it->second;
    }
};

} // namespace swan