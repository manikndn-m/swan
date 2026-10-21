#include "parser.h"

#include "ast.h"
#include "util.h"

namespace swan {

Path Parser::parse_path() {
    expect(tok_id);

    Path path;
    std::vector<StringRef> items;
    while (tok.kind == tok_id) {
        items.push_back(consume_id());
        if (tok.kind == tok_dot) {
            next();  // Skip the '.'
        }
    }

    path.items = arena.vec_to_span(items);
    return path;
}

AstNode* Parser::parse_expr() {
    return parse_assign();
}

BinOp tok_to_binop(TokenKind k) {
    switch (k) {
#define BO(X, Y) \
    case Y:      \
        return X;
#include "binops.inc"
#undef BO
        default:
            DP("Here");
            fixme();
    }

    return BinOp(0);
}

bool tok_is_one_of(TokenKind k, std::vector<TokenKind> v) {
    return std::find(v.begin(), v.end(), k) != v.end();
}

#define OPFUNC(FN, NEXTFN, TOKS)                                \
    AstNode* Parser::FN() {                                     \
        auto result = NEXTFN();                                 \
        while (tok_is_one_of(tok.kind, TOKS)) {                 \
            auto left = result;                                 \
            result = arena.alloc<BinExpr>();                    \
            result->as<BinExpr>()->op = tok_to_binop(tok.kind); \
            result->as<BinExpr>()->left = left;                 \
            next();                                             \
            result->as<BinExpr>()->right = NEXTFN();            \
        }                                                       \
                                                                \
        return result;                                          \
    }

OPFUNC(parse_assign, parse_or, {tok_assign})
OPFUNC(parse_or, parse_and, {tok_or})
OPFUNC(parse_and, parse_btor, {tok_and})
OPFUNC(parse_btor, parse_btxor, {tok_btor})
OPFUNC(parse_btxor, parse_btand, {tok_btxor})
OPFUNC(parse_btand, parse_equnequ, {tok_btand})

std::vector<TokenKind> equnequ{tok_equals, tok_nequals};
OPFUNC(parse_equnequ, parse_rel, equnequ)

std::vector<TokenKind> rel{tok_gt, tok_gteq, tok_lt, tok_lteq};
OPFUNC(parse_rel, parse_addsub, rel)

std::vector<TokenKind> addsub{tok_plus, tok_minus};
OPFUNC(parse_addsub, parse_muldiv, addsub)

std::vector<TokenKind> muldiv{tok_mul, tok_div};
OPFUNC(parse_muldiv, parse_prefix, muldiv)

AstNode* Parser::parse_prefix() {
    UnaryExpr* expr = nullptr;

    if (tok.kind == tok_not || tok.kind == tok_plus || tok.kind == tok_minus) {
        expr = arena.alloc<UnaryExpr>();
    }

    while (tok.kind == tok_not || tok.kind == tok_plus ||
           tok.kind == tok_minus) {
        next();
        expr->operand = parse_prefix();
    }

    if (expr) return expr;

    return parse_term();
}

AstNode* Parser::parse_term() {
    AstNode* result = nullptr;
    switch (tok.kind) {
        case tok_intlit: {
            auto expr = arena.alloc<IntLitExpr>();
            expr->val = tok.ival;
            next();
            result = expr;
            break;
        }
        case tok_id: {
            auto expr = arena.alloc<IdExpr>();
            expr->id = arena.alloc_str(tok.idval);
            next();
            result = expr;
            break;
        }
        case tok_oparen: {
            next();  // Skip the '('
            result = parse_expr();
            expect_and_skip(tok_cparen);
            break;
        }
        default: {
        }
    }

    while (tok.kind == tok_dot || tok.kind == tok_oparen) {
        if (tok.kind == tok_dot) {
            while (tok.kind == tok_dot) {
                BinExpr* expr = arena.alloc<BinExpr>();
                expr->op = bo_dot;
                expr->left = result;
                next();  // Skip the dot.
                expect(tok_id);
                auto idnode = arena.alloc<IdExpr>();
                idnode->id = consume_id();
                expr->right = idnode;
                result = expr;
            }
        } else if (tok.kind == tok_oparen) {
            while (tok.kind == tok_oparen) {
                next();  // Skip the '('

                std::vector<AstNode*> args;
                auto funccall = arena.alloc<FuncCallExpr>();
                funccall->callee = result;
                while (true) {
                    if (tok.kind == tok_cparen) {
                        break;
                    }

                    args.push_back(parse_expr());
                    if (tok.kind == tok_comma) {
                        next();  // Skip the comma
                    }
                }

                expect_and_skip(tok_cparen);
                funccall->args = arena.vec_to_span(args);
                result = funccall;
            }
        }
    }

    return result;
}

FunctionSignatureDef Parser::parse_func_sign() {
    FunctionSignatureDef sign;
    expect_and_skip(tok_oparen);

    std::vector<ParamDef*> params;
    while (tok.kind != tok_cparen) {
        ParamDef* param = arena.alloc<ParamDef>();
        param->name = consume_id();
        expect_and_skip(tok_colon);
        param->type_ann = parse_typeann();

        if (tok.kind == tok_comma) {
            next();
        }

        if (eof()) {
            err("unexpected EOF");
        }

        params.push_back(param);
    }

    expect_and_skip(tok_cparen);
    sign.params = arena.vec_to_span(params);

    if (tok.kind == tok_colon) {
        next();  // Skip the :
        sign.return_type_ann = parse_typeann();
    } else {
        sign.return_type_ann = nullptr;
    }

    return sign;
}

FunctionProtoDef Parser::parse_func_proto() {
    FunctionProtoDef proto;
    if (tok.kind == tok_oparen) {
        proto.has_receiver = true;
        expect_and_skip(tok_oparen);
        proto.receiver = parse_typeann();
        expect_and_skip(tok_cparen);
        proto.func_name = consume_id();
    } else {
        proto.has_receiver = false;
        proto.func_name = consume_id();
    }

    proto.sign = parse_func_sign();
    return proto;
}

AstNode* Parser::parse_stmt() {
    switch (tok.kind) {
        case tok_import: {
            next();  // Skip the 'import'
            auto stmt = arena.alloc<ImportStmt>();
            stmt->path = parse_path();
            return stmt;
        }
        case tok_struct:
        case tok_object: {
            auto stmt = arena.alloc<StructDefStmt>();
            stmt->is_object = tok.kind == tok_object;
            next();  // Skip the 'struct' or 'object'
            stmt->name = consume_id();
            expect_and_skip(tok_obrace);
            ignore_newlines();

            std::vector<FieldDef*> fields;
            while (tok.kind != tok_cbrace) {
                auto field = arena.alloc<FieldDef>();

                field->name = consume_id();
                expect_and_skip(tok_colon);

                field->type_ann = parse_typeann();
                fields.push_back(field);
                ignore_newlines();
            }

            expect_and_skip(tok_cbrace);
            ignore_newlines();
            stmt->fields = arena.vec_to_span(fields);
            return stmt;
        }
        case tok_var: {
            std::vector<VarDef*> vardefs;
            next();  // Skip the var.
            expect(tok_id);
            for (;;) {
                VarDef* var = arena.alloc<VarDef>();
                var->type_ann = nullptr;
                var->init = nullptr;
                var->id = consume_id();

                if (tok.kind == tok_assign) {
                    next();  // Skip the '='
                    var->init = parse_expr();
                }

                vardefs.push_back(var);
                if (tok.kind == tok_comma) {
                    next();
                }

                if (tok.kind != tok_id) {
                    break;
                }
            }

            TypeAnnot* typeann = nullptr;
            if (tok.kind == tok_colon) {
                next();  // Skip the ':'
                typeann = parse_typeann();
            }

            for (auto& vardef : vardefs) {
                vardef->type_ann = typeann;
            }

            if (tok.kind == tok_assign) {
                next();  // Skip the '='
                vardefs[vardefs.size() - 1]->init = parse_expr();
            }

            auto stmt = arena.alloc<VarDefStmt>();
            stmt->vardefs = arena.vec_to_span(vardefs);
            return stmt;
        }
        case tok_def: {
            next();  // Skip the def
            auto stmt = arena.alloc<FuncDefStmt>();
            stmt->proto = parse_func_proto();
            stmt->block = parse_brace_block();
            return stmt;
        }

        default:
            // Parse an expression statement.
            auto stmt = arena.alloc<ExprStmt>();
            stmt->expr = parse_expr();
            return stmt;
    }

    fixme();
}

TypeAnnot* Parser::parse_typeann() {
    TypeAnnot* ann = arena.alloc<TypeAnnot>();
    switch (tok.kind) {
        case tok_uint16:
            ann->kind = ta_uint16;
            next();
            break;
        case tok_uint32:
            ann->kind = ta_uint32;
            next();
            break;
        case tok_uint64:
            ann->kind = ta_uint64;
            next();
            break;

        case tok_int16:
            ann->kind = ta_int16;
            next();
            break;

        case tok_int32:
            ann->kind = ta_int32;
            next();
            break;

        case tok_int64:
            ann->kind = ta_int64;
            next();
            break;

        case tok_bool:
            ann->kind = ta_int32;  // FIXME: Use a proper bool type.
            next();
            break;

        case tok_shared:
            ann->kind = ta_shared;
            next();
            ann->of = parse_typeann();
            break;

        case tok_btand:
            ann->kind = ta_alias;
            next();
            if (tok.kind == tok_mut) {
                next();  // Skip the 'mut'
                ann->is_mut = true;
            }
            ann->of = parse_typeann();
            break;

        case tok_id:
            ann->kind = ta_unresolved;
            ann->unresolved = parse_path();
            break;

        default:
            fixme();
    }

    return ann;
}

Block* Parser::parse_block(TokenKind endtok) {
    Block* block = arena.alloc<Block>();
    std::vector<AstNode*> stmts;

    while (tok.kind != endtok) {
        while (tok.kind == tok_linebreak) {
            next();
        }
        if (tok.kind == endtok) {
            break;
        }
        if (tok.kind == tok_eof) {
            err("unexpected eof");
        }
        auto stmt = parse_stmt();
        stmts.push_back(stmt);

        while (tok.kind == tok_linebreak) {
            next();
        }
    }

    block->stmts = arena.vec_to_span(stmts);
    return block;
}

Block* Parser::parse() {
    // Read the first token into 'tok'.
    next();
    return parse_block(tok_eof);
}

}  // namespace swan
