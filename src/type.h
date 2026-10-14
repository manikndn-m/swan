#pragma once

#include "ast.h"

namespace swan {

enum TypeKind {
    tk_int16,
    tk_int32,
    tk_int64,

    tk_uint16,
    tk_uint32,
    tk_uint64,

    tk_alias,
    tk_array,
    tk_ref,
    tk_shared,

    tk_struct,
    tk_interface,
};

struct Struct;

struct Type {
    TypeKind kind;
    Struct *struct_; // Applies if 'kind' is tk_struct
    int bits;        // Applies if 'kind' is one of Int, Uint and Float.

    Type *of; // Applies if 'kind' is one of Alias, Array.
};

} // namespace swan