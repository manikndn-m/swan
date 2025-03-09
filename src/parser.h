#pragma once

#include "ast.h"
#include "scanner.h"
#include "util.h"
#include <cstdlib>

namespace swan {

struct ParserError {
    std::string msg;
    SourcePos pos;
};

class Parser {
    Scanner s;
    Token tok;
    Arena *ast_arena;

    template <typename T>
    T *allocnode()
    {
        return (T *)ast_arena->allocate(sizeof(T));
    }

    template <typename T>
    Span<T *> vec_to_span(const std::vector<T *> &v)
    {
        T *ptr = (T *)ast_arena->allocate(sizeof(T) * v.size);

        return Span<T *>(ptr);
    }

    void next()
    {
        s.next(tok);
    }

    bool eof() const
    {
        return tok.kind == tok_eof;
    }

  public:
    Parser(std::string program, int fileid, std::string filename,
           Arena *ast_arena)
        : s(program, fileid, filename)
    {
        this->ast_arena = ast_arena;
    }

    Block *parse();

  private:
    Block *parse_block(TokenKind endtok);
    AstNode *parse_stmt();
};

} // namespace swan