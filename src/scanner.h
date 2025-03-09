#pragma once

#include "token.h"
#include <functional>
#include <string>

namespace swan {

struct ScannerError {
    std::string msg;
    SourcePos pos;
};

class Scanner {
    std::string program, filename;
    std::string::iterator it, end;
    SourcePos pos;
    std::string accum, accum2;
    std::unordered_map<std::string, TokenKind> keywordmap;

    bool eof() const
    {
        return it >= end;
    }

    void advance()
    {
        it++;
        pos.col++;
    }

    static bool is_whitespace(char c)
    {
        return c == ' ' || c == '\t';
    }

    int parse_num()
    {
        accum2.clear();
        take_while(accum, [](char c) { return std::isdigit(c); });
        int num = std::stoi(accum2);
        return num;
    }

    template <typename... Args>
    void err(std::string msg)
    {
        ScannerError e;
        e.msg = msg;
        e.pos = pos;
        throw e;
    }

    void take_while(std::string &acc, std::function<bool(char)> pred);

  public:
    Scanner(std::string program, int fileid, std::string filename)
    {
        this->program = program;
        this->filename = filename;
        this->it = this->program.begin();
        this->end = this->program.end();
        pos.fileid = fileid;
#define K(X, Y) keywordmap[Y] = X;
#include "keywords.inc"
#undef K
    }

    void next(Token &out);
};

} // namespace swan