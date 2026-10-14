#pragma once

#include "ast.h"
#include "scanner.h"
#include "util.h"
#include <cstdlib>
#include <cstring>

namespace swan {

struct ParserError {
    std::string msg;
    SourcePos pos;
};

class Parser {
    Scanner s;
    Token tok;
    Arena *ast_arena;

    template <typename T> T *alloc()
    {
        auto ptr = (T *)ast_arena->allocate(sizeof(T));
        new (ptr) T();
        return ptr;
    }

    template <typename T> Span<T> vec_to_span(const std::vector<T> &v)
    {
        T *ptr = (T *)ast_arena->allocate(sizeof(T) * v.size());
        memcpy(ptr, v.data(), sizeof(T) * v.size());
        return Span<T>(ptr, v.size());
    }

    StringRef alloc_str(std::string str)
    {
        char *ptr = (char *)ast_arena->allocate(sizeof(char) * str.size());
        memcpy(ptr, str.data(), sizeof(char) * str.size());
        return StringRef(ptr, str.size());
    }

    StringRef consume_id()
    {
        expect(tok_id);
        StringRef sref = alloc_str(tok.idval);
        next(); // Skip the identifier
        return sref;
    }

    template <typename... Args> void err(std::string msg)
    {
        ParserError e;
        e.msg = msg;
        e.pos = s.curpos();
        std::cout << msg << std::endl;
        throw e;
    }

    void expect(TokenKind k)
    {
        if (tok.kind != k) {
            Token expected = Token{.kind = k};
            err("expected a '" + expected.to_string() + "' but found a " +
                tok.to_string());
        }
    }

    void expect_and_skip(TokenKind k)
    {
        expect(k);
        next();
    }

    void ignore_newlines()
    {
        while (tok.kind == tok_linebreak) {
            next();
        }
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
    Path parse_path();
    AstNode *parse_stmt();
    Block *parse_block(TokenKind endtok);

    AstNode *parse_expr();

    AstNode *parse_assign();
    AstNode *parse_or();
    AstNode *parse_and();
    AstNode *parse_btor();
    AstNode *parse_btxor();
    AstNode *parse_btand();
    AstNode *parse_equnequ();
    AstNode *parse_rel();
    AstNode *parse_addsub();
    AstNode *parse_muldiv();
    AstNode *parse_prefix();
    AstNode *parse_term();

    TypeAnnot *parse_typeann();
    FunctionSignatureDef parse_func_sign();
    FunctionProtoDef parse_func_proto();

    Block *parse_brace_block()
    {
        ignore_newlines();
        expect_and_skip(tok_obrace);
        ignore_newlines();

        auto result = parse_block(tok_cbrace);

        ignore_newlines();
        expect_and_skip(tok_cbrace);
        ignore_newlines();

        return result;
    }
};

} // namespace swan