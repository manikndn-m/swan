#pragma once

#include "token.h"
#include "util.h"

namespace swan {

struct Path {
    Span<StringRef> items;
};

enum TypeAnnotKind {};

struct TypeAnnot {
    TypeAnnotKind kind;
};

enum AstNodeKind {
    ak_import,
};

struct AstNode {
    AstNodeKind kind;
    SourcePos pos;

    AstNode(AstNodeKind kind) : kind(kind) {}
};

#define AST_NODE(T, K)                                                         \
    T() : AstNode(K) {}

struct ImportStmt : AstNode {
    Path path;
    bool as_specified;
    StringRef alias;

    AST_NODE(ImportStmt, ak_import);
};

struct Block {
    Span<AstNode *> stmts;
};

} // namespace swan