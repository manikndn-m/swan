#include "type.h"

#include "diag.h"
#include "symbol.h"

namespace swan {

Type *type_ann_to_type(TypeAnnot *typeann, Scope *scope, Arena &arena)
{
    auto primty = [](TypeKind k) {
        static Type type = Type{.kind = k};
        return &type;
    };

    switch (typeann->kind) {
    case ta_bool:
        return primty(tk_bool);
    case ta_uint16:
        return primty(tk_uint16);
    case ta_uint32:
        return primty(tk_uint32);
    case ta_uint64:
        return primty(tk_uint64);
    case ta_int16:
        return primty(tk_int16);
    case ta_int32:
        return primty(tk_int32);
    case ta_int64:
        return primty(tk_int64);

    case ta_shared: {
        auto type = arena.alloc<Type>();
        type->kind = tk_shared;
        type->of = type_ann_to_type(typeann->of, scope, arena);
        return type;
    }

    case ta_alias: {
        auto type = arena.alloc<Type>();
        type->kind = tk_alias;
        type->of = type_ann_to_type(typeann->of, scope, arena);
        type->mutable_ = typeann->is_mut;
        return type;
    }

    case ta_unresolved: {
        auto type = arena.alloc<Type>();
        for (auto item : typeann->unresolved.items) {
            auto sym = scope->find(item);
            if (sym == std::nullopt) {
                err("cannot find symbol ", item);
            }

            if (std::holds_alternative<Struct *>(*sym)) {
                Struct *struct_ = std::get<Struct *>(*sym);
                delete type; // Sorry!
                type = struct_->struct_type;
            } else {
                fixme(); // TODO: handle imported modules, interfaces etc.,
            }
        }
        return type;
    }

    default:
        break;
    }

    fixme();
    return nullptr;
}

StringRef type_to_ctype(Type *type, Scope *scope, Arena &arena)
{
    switch (type->kind) {
    case tk_uint16:
        return "uint16_t";
    case tk_uint32:
        return "uint32_t";
    case tk_uint64:
        return "uint64_t";
    case tk_int16:
        return "int16_t";
    case tk_int32:
        return "int32_t";
    case tk_int64:
        return "int64_t";
    case tk_struct:
        return type->struct_->canon_name;
    case tk_alias:
        return arena.conc_srefs(type_to_ctype(type->of, scope, arena), " *");
    case tk_shared:
        return arena.conc_srefs(type_to_ctype(type->of, scope, arena), " *");
    default:
        fixme();
    }

    return "unreachable";
}

Type *infer_type(AstNode *expr, Scope *scope, Arena &arena)
{
    auto primty = [](TypeKind kind) {
        Type *type = new Type();
        type->kind = kind;
        return type;
    };

    switch (expr->kind) {
    case ak_intlit: {
        auto intexpr = expr->as<IntLitExpr>();
        (void)intexpr;
        return primty(tk_int32);
    }
    case ak_id: {
        Type *type = nullptr;
        auto idexpr = expr->as<IdExpr>();
        auto sym = scope->find(idexpr->id);
        if (sym == std::nullopt) {
            err("cannot find symbol ", idexpr->id);
        }

        if (std::holds_alternative<Var *>(*sym)) {
            Var *var = std::get<Var *>(*sym);
            type = var->type;
        } else if (std::holds_alternative<Struct *>(*sym) ||
                   std::holds_alternative<Function *>(*sym)) {
            err("cannot use a struct or function as an expression");
        } else {
            fixme();
        }

        return type;
    }

    default:
        break;
    }

    fixme();
}

} // namespace swan
