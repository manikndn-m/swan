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

#define STMT_NODE(T, K)                                                        \
    T() : StmtNode(K) {}

struct StmtNode : AstNode {
    StmtNode *next = nullptr;
    StmtNode(AstNodeKind kind) : AstNode(kind) {}
};

struct ImportStmt : StmtNode {
    Path path;
    bool as_specified;
    StringRef alias;

    STMT_NODE(ImportStmt, ak_import);
};

struct Block {
    StmtNode *first;
};

} // namespace swan