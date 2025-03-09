#pragma once

#include "ast.h"
#include "scanner.h"
#include "util.h"

namespace swan {

struct ParserError {
    std::string msg;
    SourcePos pos;
};

class Parser {
    Scanner s;
    Token tok;

  public:
    Parser(std::string program, int fileid, std::string filename)
        : s(program, fileid, filename)
    {
    }

    Block *parse();
};

} // namespace swan