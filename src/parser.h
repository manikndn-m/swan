#pragma once

#include <cstdlib>
#include <cstring>

#include "arena.h"
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
    Arena& arena;

    StringRef consume_id() {
        expect(tok_id);
        StringRef sref = arena.alloc_str(tok.idval);
        next();  // Skip the identifier
        return sref;
    }

    template <typename... Args>
    void err(std::string msg) {
        ParserError e;
        e.msg = msg;
        e.pos = s.curpos();
        std::cout << msg << std::endl;
        throw e;
    }

    void expect(TokenKind k) {
        if (tok.kind != k) {
            Token expected = Token{.kind = k};
            err("expected a '" + expected.to_string() + "' but found a " +
                tok.to_string());
        }
    }

    void expect_and_skip(TokenKind k) {
        expect(k);
        next();
    }

    void ignore_newlines() {
        while (tok.kind == tok_linebreak) {
            next();
        }
    }

    void next() {
        s.next(tok);
    }

    bool eof() const {
        return tok.kind == tok_eof;
    }

   public:
    Parser(std::string program, int fileid, std::string filename,
           Arena& ast_arena)
        : s(program, fileid, filename), arena(ast_arena) {
    }

    Block* parse();

   private:
    Path parse_path();
    AstNode* parse_stmt();
    Block* parse_block(TokenKind endtok);

    AstNode* parse_expr();

    AstNode* parse_assign();
    AstNode* parse_or();
    AstNode* parse_and();
    AstNode* parse_btor();
    AstNode* parse_btxor();
    AstNode* parse_btand();
    AstNode* parse_equnequ();
    AstNode* parse_rel();
    AstNode* parse_addsub();
    AstNode* parse_muldiv();
    AstNode* parse_prefix();
    AstNode* parse_term();

    TypeAnnot* parse_typeann();
    FunctionSignatureDef parse_func_sign();
    FunctionProtoDef parse_func_proto();

    Block* parse_brace_block() {
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

}  // namespace swan