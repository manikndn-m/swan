#pragma once

#include "token.h"
#include "util.h"

namespace swan {

struct Path {
    Span<StringRef> items;
};

enum TypeAnnotKind {
    ta_int16,
    ta_int32,
    ta_int64,

    ta_uint16,
    ta_uint32,
    ta_uint64,

    ta_alias,
    ta_array,
    ta_shared,

    ta_unresolved,
};

enum Visibility {
    vi_public,
    vi_private,
};

struct TypeAnnot {
    TypeAnnotKind kind;
    TypeAnnot *of;

    Path unresolved;
};

enum AstNodeKind {
    ak_binexpr,
    ak_intlit,
    ak_floatlit,
    ak_strlit,
    ak_listlit,
    ak_id,
    ak_funccall,
    ak_unary,

    ak_import,
    ak_vardef,
    ak_vardefstmt,
    ak_fielddef,
    ak_structdef,
    ak_paramdef,
    ak_funcproto,
    ak_funcsig,
    ak_funcdef,
    ak_exprstmt,

    ak_typeannot,
};

struct AstNode {
    AstNodeKind kind;
    SourcePos pos;
    Visibility visibility;

    AstNode(AstNodeKind kind) : kind(kind) {
    }

    virtual void dummy() {
    }

    template <typename T>
    T *as() {
        auto ptr = dynamic_cast<T *>(this);
        if (ptr == nullptr) {
            panic("casting to the wrong type");
        }
        return ptr;
    }
};

#define AST_NODE(T, K)                                                         \
    T() : AstNode(K) {                                                         \
    }

enum UnaryOp {
    uno_not,
    uno_negate,
};

enum BinOp {

#define BO(X, Y) X,
#include "binops.inc"
#undef BO

};

#define N(T, K)                                                                \
    struct T : AstNode {                                                       \
        AST_NODE(T, K);
#define F(T, N) T N;
#define NEND                                                                   \
    }                                                                          \
    ;

#include "nodedefs.inc"
#undef N
#undef F
#undef NEND

struct ParamDef : AstNode {
    StringRef name;
    TypeAnnot *type_ann;

    AST_NODE(ParamDef, ak_paramdef);
};

struct FunctionSignatureDef : AstNode {
    Span<ParamDef> params;
    TypeAnnot *return_type_ann;

    AST_NODE(FunctionSignatureDef, ak_funcsig);
};

struct FunctionProtoDef : AstNode {
    bool has_receiver;
    TypeAnnot *receiver;
    StringRef func_name;
    FunctionSignatureDef sign;

    AST_NODE(FunctionProtoDef, ak_funcproto);
};

struct Block;

struct FuncDefStmt : AstNode {
    FunctionProtoDef proto;
    Block *block;

    AST_NODE(FuncDefStmt, ak_funcdef);
};

struct ExprStmt : AstNode {
    AstNode *expr;

    AST_NODE(ExprStmt, ak_exprstmt);
};

struct Block {
    Span<AstNode *> stmts;
};

} // namespace swan