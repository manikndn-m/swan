#include <functional>
#include <iostream>
#include <vector>

#include "arena.h"
#include "ast.h"
#include "diag.h"
#include "parser.h"
#include "scanner.h"

using namespace swan;

// the "expect" macro...
#define EXPECT(C)                                                              \
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
        EXPECT(check_kinds(
            vec, {tok_if, tok_intlit, tok_obrace, tok_intlit, tok_cbrace}))
        EXPECT(vec[3].ival == 42);
    }

    {
        TOKENIZE(" \"wor\\nld\" 'a'");
        EXPECT(check_kinds(vec, {tok_strlit, tok_charlit}));
        EXPECT(vec[0].strval == "wor\nld");
        EXPECT(vec[1].charval == 'a');
    }

    {
        TOKENIZE("import std.io // comment\n");
        EXPECT(check_kinds(
            vec, {tok_import, tok_id, tok_dot, tok_id, tok_linebreak}));
        EXPECT(vec[1].idval == "std");
        EXPECT(vec[3].idval == "io");
    }
}

void parser_tests()
{
    Arena arena;
#define PARSE(CODE)                                                            \
    Parser p(CODE, 0, "<test>", arena);                                        \
    auto block = p.parse();                                                    \
    auto stmts = block->stmts;

    {
        PARSE(" import std.io \n import std.log\na.b");

        EXPECT(stmts[0]->kind == ak_import &&
               stmts[0]->as<ImportStmt>()->path.items[0] == "std" &&
               stmts[0]->as<ImportStmt>()->path.items[1] == "io");

        EXPECT(stmts[1]->kind == ak_import &&
               stmts[1]->as<ImportStmt>()->path.items[0] == "std" &&
               stmts[1]->as<ImportStmt>()->path.items[1] == "log");

        EXPECT(stmts[2]->kind == ak_exprstmt);
        auto expr = stmts[2]->as<ExprStmt>()->expr;
        EXPECT(expr->kind == ak_binexpr);
    }
    {
        PARSE("foo(42, a).a");
        EXPECT(stmts.size() == 1);
        auto expr = stmts[0]->as<ExprStmt>()->expr;
        EXPECT(expr->kind == ak_binexpr);

        auto binexpr = expr->as<BinExpr>();

        auto fc = binexpr->left->as<FuncCallExpr>();
        EXPECT(fc->args.size() == 2);

        EXPECT(fc->args[0]->as<IntLitExpr>()->val == 42);
        EXPECT(fc->args[1]->as<IdExpr>()->id == "a");

        EXPECT(binexpr->left->kind == ak_funccall);
        EXPECT(binexpr->right->kind == ak_id);
    }

    {
        PARSE("a.foo(42, a)");
        EXPECT(stmts.size() == 1);
        auto expr = stmts[0]->as<ExprStmt>()->expr;
        EXPECT(expr->kind == ak_funccall);

        auto fc = expr->as<FuncCallExpr>();
        EXPECT(fc->args.size() == 2);
        EXPECT(fc->callee->kind == ak_binexpr);

        auto binexpr = fc->callee->as<BinExpr>();

        EXPECT(binexpr->left->kind == ak_id);
        EXPECT(binexpr->right->kind == ak_id);
        EXPECT(binexpr->left->as<IdExpr>()->id == "a");
        EXPECT(binexpr->right->as<IdExpr>()->id == "foo");

        EXPECT(fc->args[0]->as<IntLitExpr>()->val == 42);
        EXPECT(fc->args[1]->as<IdExpr>()->id == "a");
    }
    {
        PARSE("var a: int32 = 3");
        EXPECT(stmts.size() == 1);
        EXPECT(stmts[0]->kind == ak_vardefstmt);

        auto stmt = stmts[0]->as<VarDefStmt>();
        EXPECT(stmt->vardefs.size() == 1)
        EXPECT(stmt->vardefs[0]->type_ann->kind == ta_int32);
    }

    {
        PARSE("struct Foo {\n"
              "  s: int32\n"
              "}\n"
              "\n"
              "var a: int32 = 10\n"
              "var b: int32 = 16\n"
              "var c = 3\n");
        EXPECT(stmts.size() == 4);

        auto struct_stmt = stmts[0]->as<StructDefStmt>();
        EXPECT(struct_stmt->name == "Foo");
        EXPECT(struct_stmt->fields.size() == 1);
        EXPECT(struct_stmt->fields[0]->name == "s");
        EXPECT(struct_stmt->fields[0]->type_ann->kind == ta_int32);

        auto var_a = stmts[1]->as<VarDefStmt>()->vardefs[0];
        EXPECT(var_a->id == "a");
        EXPECT(var_a->type_ann->kind == ta_int32);
        EXPECT(var_a->init->as<IntLitExpr>()->val == 10);

        auto var_b = stmts[2]->as<VarDefStmt>()->vardefs[0];
        EXPECT(var_b->id == "b");
        EXPECT(var_b->type_ann->kind == ta_int32);
        EXPECT(var_b->init->as<IntLitExpr>()->val == 16);

        auto var_c = stmts[3]->as<VarDefStmt>()->vardefs[0];
        EXPECT(var_c->id == "c");
        EXPECT(var_c->type_ann == nullptr);
        EXPECT(var_c->init->as<IntLitExpr>()->val == 3);
    }

    {
        PARSE("var a: Foo\nvar b: &Bar");
        EXPECT(stmts.size() == 2);
        EXPECT(stmts[0]->kind == ak_vardefstmt);

        auto stmt = stmts[0]->as<VarDefStmt>();
        EXPECT(stmt->vardefs.size() == 1)
        EXPECT(stmt->vardefs[0]->type_ann->kind == ta_unresolved);
        EXPECT(stmt->vardefs[0]->type_ann->unresolved.items[0] == "Foo");

        stmt = stmts[1]->as<VarDefStmt>();
        EXPECT(stmt->vardefs.size() == 1)
        EXPECT(stmt->vardefs[0]->type_ann->kind == ta_alias);
        EXPECT(stmt->vardefs[0]->type_ann->of->unresolved.items[0] == "Bar");
    }

    {
        PARSE("struct Foo { a: int32\n b : &Foo}")

        EXPECT(stmts.size() == 1);
        auto stmt = stmts[0]->as<StructDefStmt>();
        EXPECT(stmt->name == "Foo");
        EXPECT(stmt->fields.size() == 2);

        auto fields = stmt->fields;
        EXPECT(fields[0]->name == "a");
        EXPECT(fields[0]->type_ann->kind == ta_int32);
        EXPECT(fields[1]->name == "b");
        EXPECT(fields[1]->type_ann->kind == ta_alias);
        EXPECT(fields[1]->type_ann->of->unresolved.items[0] == "Foo");
    }

    {
        PARSE("\nstruct Foo {\n a: int32\n}")

        EXPECT(stmts.size() == 1);
        auto stmt = stmts[0]->as<StructDefStmt>();
        EXPECT(stmt->fields.size() == 1);
        EXPECT(stmt->fields[0]->name == "a");
        EXPECT(stmt->fields[0]->type_ann->kind == ta_int32);
    }

    {
        // FIXME
        PARSE("def foo(renderer: &mut re.Renderer, x: int) { \
                        var bar: int \n\
                        print() \
            }");

        EXPECT(stmts.size() == 1 && stmts[0]->kind == ak_funcdef);

        auto funcdefstmt = stmts[0]->as<FuncDefStmt>();
        EXPECT(funcdefstmt->proto.sign.params.size() == 2);
        EXPECT(funcdefstmt->proto.sign.params[0]->name == "renderer");
        EXPECT(funcdefstmt->proto.sign.params[1]->name == "x");

        auto typeann = funcdefstmt->proto.sign.params[0]->type_ann;
        EXPECT(typeann->kind == ta_alias &&
               typeann->of->unresolved.items[0] == "re" &&
               typeann->of->unresolved.items[1] == "Renderer" &&
               typeann->is_mut == true);

        auto fblock = funcdefstmt->block;
        EXPECT(fblock->stmts.size() == 2);
        EXPECT(fblock->stmts[0]->as<VarDefStmt>()->vardefs[0]->id == "bar");

        auto expr = fblock->stmts[1]->as<ExprStmt>()->expr;
        EXPECT(expr->kind == ak_funccall);
        auto callexpr = expr->as<FuncCallExpr>();
        EXPECT(callexpr->callee->kind == ak_id);
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