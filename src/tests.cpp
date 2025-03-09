#include "ast.h"
#include "diag.h"
#include "parser.h"
#include "scanner.h"
#include <functional>
#include <iostream>
#include <vector>

using namespace swan;

// the "expect" macro...
#define EXP(C)                                                                 \
    {                                                                          \
        if (!(C)) {                                                            \
            std::cout << "Condition at line " << __LINE__ << " failed.\n";     \
            std::exit(0);                                                      \
        }                                                                      \
    }

void scanner_tests()
{
#define TOKENIZE(STR)                                                          \
    Scanner s(STR, 0, "<test-file>");                                          \
    std::vector<Token> vec;                                                    \
    Token tok;                                                                 \
    s.next(tok);                                                               \
    try {                                                                      \
        while (tok.kind != tok_eof) {                                          \
            vec.push_back(tok);                                                \
            s.next(tok);                                                       \
        }                                                                      \
    } catch (ScannerError e) {                                                 \
        error(e.pos, e.msg);                                                   \
        std::exit(-1);                                                         \
    }

    auto check_kinds = [](std::vector<Token> v, std::vector<TokenKind> kinds) {
        if (v.size() != kinds.size()) {
            return false;
        }

        for (auto i = 0; i < v.size(); i++) {
            if (v[i].kind != kinds[i]) {
                return false;
            }
        }

        return true;
    };

    {
        TOKENIZE("hello 3.14 if  &&|| \"world\";\n :) { } :D ;)");
        EXP(vec.size() == 16);

        // Basic sanity check...
        //   for (auto tok : vec) {
        //      tok.dbgprint();
        // }
    }

    {
        TOKENIZE("if 2 {42} ");
        EXP(check_kinds(
            vec, {tok_if, tok_intlit, tok_obrace, tok_intlit, tok_cbrace}))
        EXP(vec[3].ival == 42);
    }

    {
        TOKENIZE(" \"wor\\nld\" ");
        EXP(check_kinds(vec, {tok_strlit}));
        EXP(vec[0].strval == "wor\nld");
    }
}

void parser_tests()
{
    {
    }
}

int main()
{
    scanner_tests();
    parser_tests();

    std::cout << "All tests passed.\n";
    return 0;
}