#pragma once

#include <cstdint>
#include <iostream>
#include <string>

namespace swan {

struct SourcePos {
    int line, col;
    int fileid;

    SourcePos() : line(1), col(1)
    {
    }
};

enum TokenKind {
    tok_id,
    tok_strlit,
    tok_charlit,
    tok_floatlit,
    tok_intlit,
    tok_linebreak,

    tok_opd_start_,
#define OPD(X, Y) X,
#include "opdelims.inc"
#undef OPD
    tok_opd_end_,

    tok_keyword_start_,
#define K(X, Y) X,
#include "keywords.inc"
#undef K
    tok_keyword_end_,

    tok_eof,
};

struct nlsuffix {
    bool signedness;
    bool floating;
    int bits;
};

struct Token {
    SourcePos start_pos, end_pos;
    TokenKind kind;
    std::string strval, idval;
    double fval;
    uint64_t ival;

    std::string to_string() const;

    void dbgprint()
    {
        std::cout << to_string() << "\n";
    }
};

} // namespace swan