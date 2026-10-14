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
            std::cout << "\nCondition '" << #C << "' at line " << __LINE__     \
                      << " failed.\n";                                         \
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

        for (size_t i = 0; i < v.size(); i++) {
            if (v[i].kind != kinds[i]) {
                return false;
            }
        }

        return true;
    };

    {
        TOKENIZE("import std.io\n a ");
        //        EXP(vec.size() == 16);

        // Basic sanity check...
        /* for (auto tok : vec) {
             tok.dbgprint();
         }
        */
    }

    {
        TOKENIZE("if 2 {42} ");
        EXP(check_kinds(
            vec, {tok_if, tok_intlit, tok_obrace, tok_intlit, tok_cbrace}))
        EXP(vec[3].ival == 42);
    }

    {
        TOKENIZE(" \"wor\\nld\" 'a'");
        EXP(check_kinds(vec, {tok_strlit, tok_charlit}));
        EXP(vec[0].strval == "wor\nld");
        EXP(vec[1].charval == 'a');
    }

    {
        TOKENIZE("import std.io");
        EXP(check_kinds(vec, {tok_import, tok_id, tok_dot, tok_id}))
        EXP(vec[1].idval == "std");
        EXP(vec[3].idval == "io");
    }
}

void parser_tests()
{
    Arena ast_arena;
#define PARSE(CODE)                                                            \
    Parser p(CODE, 0, "<test>", &ast_arena);                                   \
    auto block = p.parse();                                                    \
    auto stmts = block->stmts;

    {
        PARSE(" import std.io \n import std.log\na.b");

        EXP(stmts[0]->kind == ak_import &&
            stmts[0]->as<ImportStmt>()->path.items[0] == "std" &&
            stmts[0]->as<ImportStmt>()->path.items[1] == "io");

        EXP(stmts[1]->kind == ak_import &&
            stmts[1]->as<ImportStmt>()->path.items[0] == "std" &&
            stmts[1]->as<ImportStmt>()->path.items[1] == "log");

        EXP(stmts[2]->kind == ak_exprstmt);
        auto expr = stmts[2]->as<ExprStmt>()->expr;
        EXP(expr->kind == ak_binexpr);
        // auto bin = expr->as<BinExpr>();

        // bin->dbgprint();
    }
    {
        PARSE("foo(42, a).a");
        EXP(stmts.size() == 1);
        auto expr = stmts[0]->as<ExprStmt>()->expr;
        EXP(expr->kind == ak_binexpr);

        auto binexpr = expr->as<BinExpr>();

        auto fc = binexpr->left->as<FuncCallExpr>();
        EXP(fc->args.size() == 2);

        EXP(fc->args[0]->as<IntLitExpr>()->val == 42);
        EXP(fc->args[1]->as<IdExpr>()->id == "a");

        EXP(binexpr->left->kind == ak_funccall);
        EXP(binexpr->right->kind == ak_id);
    }

    {
        PARSE("a.foo(42, a)");
        EXP(stmts.size() == 1);
        auto expr = stmts[0]->as<ExprStmt>()->expr;
        EXP(expr->kind == ak_funccall);

        auto fc = expr->as<FuncCallExpr>();
        EXP(fc->args.size() == 2);
        EXP(fc->callee->kind == ak_binexpr);

        auto binexpr = fc->callee->as<BinExpr>();

        EXP(binexpr->left->kind == ak_id);
        EXP(binexpr->right->kind == ak_id);
        EXP(binexpr->left->as<IdExpr>()->id == "a");
        EXP(binexpr->right->as<IdExpr>()->id == "foo");

        EXP(fc->args[0]->as<IntLitExpr>()->val == 42);
        EXP(fc->args[1]->as<IdExpr>()->id == "a");
    }
    {
        PARSE("var a: int32 = 3");
        EXP(stmts.size() == 1);
        EXP(stmts[0]->kind == ak_vardefstmt);

        auto stmt = stmts[0]->as<VarDefStmt>();
        EXP(stmt->vardefs.size() == 1)
        EXP(stmt->vardefs[0].type_ann->kind == ta_int32);
    }

    {
        PARSE("var a: Foo\nvar b: &Bar");
        EXP(stmts.size() == 2);
        EXP(stmts[0]->kind == ak_vardefstmt);

        auto stmt = stmts[0]->as<VarDefStmt>();
        EXP(stmt->vardefs.size() == 1)
        EXP(stmt->vardefs[0].type_ann->kind == ta_unresolved);
        EXP(stmt->vardefs[0].type_ann->unresolved.items[0] == "Foo");

        stmt = stmts[1]->as<VarDefStmt>();
        EXP(stmt->vardefs.size() == 1)
        EXP(stmt->vardefs[0].type_ann->kind == ta_alias);
        EXP(stmt->vardefs[0].type_ann->of->unresolved.items[0] == "Bar");
    }

    {
        PARSE("struct Foo { a: int32\n b : &Foo}")

        EXP(stmts.size() == 1);
        auto stmt = stmts[0]->as<StructDefStmt>();
        EXP(stmt->name == "Foo");
        EXP(stmt->fields.size() == 2);

        auto fields = stmt->fields;
        EXP(fields[0].name == "a");
        EXP(fields[0].type_ann->kind == ta_int32);
        EXP(fields[1].name == "b");
        EXP(fields[1].type_ann->kind == ta_alias);
        EXP(fields[1].type_ann->of->unresolved.items[0] == "Foo");
    }

    {
        PARSE("\nstruct Foo {\n a: int32\n}")

        EXP(stmts.size() == 1);
        auto stmt = stmts[0]->as<StructDefStmt>();
        EXP(stmt->fields.size() == 1);
        EXP(stmt->fields[0].name == "a");
        EXP(stmt->fields[0].type_ann->kind == ta_int32);
    }

    {
        // FIXME
        PARSE("def foo(renderer: &re.Renderer, x: int) { \
                        var bar: int \n\
                        print() \
            }");

        EXP(stmts.size() == 1 && stmts[0]->kind == ak_funcdef);

        auto funcdefstmt = stmts[0]->as<FuncDefStmt>();
        EXP(funcdefstmt->proto.sign.params.size() == 2);
        EXP(funcdefstmt->proto.sign.params[0].name == "renderer");
        EXP(funcdefstmt->proto.sign.params[1].name == "x");

        auto typeann = funcdefstmt->proto.sign.params[0].type_ann;
        EXP(typeann->kind == ta_alias &&
            typeann->of->unresolved.items[0] == "re" &&
            typeann->of->unresolved.items[1] == "Renderer");

        auto fblock = funcdefstmt->block;
        EXP(fblock->stmts.size() == 2);
        EXP(fblock->stmts[0]->as<VarDefStmt>()->vardefs[0].id == "bar");

        auto expr = fblock->stmts[1]->as<ExprStmt>()->expr;
        EXP(expr->kind == ak_funccall);
        auto callexpr = expr->as<FuncCallExpr>();
        EXP(callexpr->callee->kind == ak_id);
    }
}

#define RUN(F)                                                                 \
    std::cout << "Running " << #F << "...";                                    \
    F();                                                                       \
    std::cout << " passed\n";

int main()
{
    RUN(scanner_tests);
    RUN(parser_tests);
    std::cout << "All tests passed.\n";
    return 0;
}