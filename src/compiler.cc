#include "compiler.h"
#include <variant>

namespace swan {

fs::path Compiler::find_module_file(Path p) {
    return "";
}

Symbol Compiler::resolve_path(Path p) {
    return Symbol();
}

Type *Compiler::type_ann_to_type(TypeAnnot *typeann, Scope *scope) {
    auto primty = [](TypeKind kind) {
        static Type type;
        type.kind = kind;
        return &type;
    };

    switch (typeann->kind) {
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
        auto type = alloc<Type>();
        type->kind = tk_shared;
        type->of = type_ann_to_type(typeann->of, scope);
        return type;
    }

    case ta_alias: {
        auto type = alloc<Type>();
        type->kind = tk_alias;
        type->of = type_ann_to_type(typeann->of, scope);
        type->mutable_ = typeann->is_mut;
        return type;
    }

    case ta_unresolved: {
        auto type = alloc<Type>();
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
    }

    default:
        break;
    }

    fixme();
    return nullptr;
}

StringRef Compiler::type_to_ctype(Type *type, Scope *scope) {
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
        return alloc_conc_strrefs(type_to_ctype(type->of, scope), " *");
    case tk_shared:
        return alloc_conc_strrefs(type_to_ctype(type->of, scope), " *");
    default:
        fixme();
    }

    return "unreachable";
}

Type *Compiler::infer_type(AstNode *expr, Scope *scope) {
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
        auto idexpr = expr->as<IdExpr>();
        auto sym = scope->find(idexpr->id);
        if (sym == std::nullopt) {
            err("cannot find symbol ", idexpr->id);
        }

        if (std::holds_alternative<Var *>(*sym)) {
            Var *var = std::get<Var *>(*sym);
            return var->type;
        } else if (std::holds_alternative<Struct *>(*sym) ||
                   std::holds_alternative<Function *>(*sym)) {
            err("cannot use a struct or function as an expression");
        } else {
            fixme();
        }
    }
    default:
        fixme();
    }
    return nullptr;
}

void Compiler::emit_expr(AstNode *expr, Scope *scope) {
    switch (expr->kind) {
    case ak_intlit: {
        auto intexpr = expr->as<IntLitExpr>();
        emit(intexpr->val);
    } break;
    case ak_id: {
        auto idexpr = expr->as<IdExpr>();

        auto sym = scope->find(idexpr->id);
        if (sym == std::nullopt) {
            err("cannot find symbol ", idexpr->id);
        }

        Var *var = std::get<Var *>(*sym);
        emit(var->canon_name);
    } break;
    default:
        fixme();
    }
}

void Compiler::add_symbol_from_stmt(AstNode *stmt, Scope *scope) {
    switch (stmt->kind) {
    case ak_structdef: {
        auto sdstmt = stmt->as<StructDefStmt>();
        Struct *struct_ = new Struct();
        for (auto fielddef : sdstmt->fields) {
            Field f;
            f.name = fielddef.name;
            f.type = type_ann_to_type(fielddef.type_ann, scope);
        }

        struct_->canon_name = alloc_conc_strrefs(module_prefix, sdstmt->name);
        struct_->struct_type = alloc<Type>();
        struct_->struct_type->kind = tk_struct;
        struct_->struct_type->struct_ = struct_;

        scope->add(sdstmt->name, struct_);
    } break;
    case ak_vardefstmt: {
        auto vdstmt = stmt->as<VarDefStmt>();

        for (auto vardef : vdstmt->vardefs) {
            Type *type = nullptr;

            if (vardef.type_ann != nullptr) {
                type = type_ann_to_type(vardef.type_ann, scope);
            } else if (vardef.init != nullptr) {
                type = infer_type(vardef.init, scope);
            } else {
                err("cannot infer type of variable ", vardef.id);
            }

            Var *var = new Var();
            var->type = type;
            if (scope == toplevel_scope) {
                var->canon_name = alloc_conc_strrefs(module_prefix, vardef.id);
            } else {
                var->canon_name = vardef.id;
            }

            scope->add(vardef.id, var);
        }
    } break;
    default:
        break;
    }
}

void Compiler::emit_block(Block *block, bool surround_with_braces) {
    if (surround_with_braces) {
        emit("{\n");
    }

    for (auto stmt : block->stmts) {
        emit_stmt(stmt);
    }

    if (surround_with_braces) {
        emit("}\n");
    }
}

void Compiler::emit_stmt(AstNode *stmt) {
    add_symbol_from_stmt(stmt, curr_scope);

    switch (stmt->kind) {
    case ak_vardefstmt: {
        auto vdstmt = stmt->as<VarDefStmt>();

        for (auto vardef : vdstmt->vardefs) {
            Type *type = nullptr;
            if (vardef.type_ann != nullptr) {
                type = type_ann_to_type(vardef.type_ann, curr_scope);
            } else if (vardef.init != nullptr) {
                auto symbol = curr_scope->find(vardef.id);
                if (symbol == std::nullopt ||
                    !std::holds_alternative<Var *>(*symbol)) {
                    err("cannot find variable ", vardef.id);
                }
                type = std::get<Var *>(*symbol)->type;
            }

            StringRef cvar_name;
            if (curr_scope == toplevel_scope) {
                cvar_name = alloc_conc_strrefs(module_prefix, vardef.id);
            } else {
                cvar_name = vardef.id;
            }

            emit(type_to_ctype(type, curr_scope), " ", cvar_name);

            if (vardef.init != nullptr) {
                emit(" = ");
                emit_expr(vardef.init, curr_scope);
            }
        }
    } break;
    case ak_structdef: {
        auto sdstmt = stmt->as<StructDefStmt>();
        emit("struct ", alloc_conc_strrefs(module_prefix, sdstmt->name),
             " {\n");

        for (auto fielddef : sdstmt->fields) {
            auto type = type_ann_to_type(fielddef.type_ann, curr_scope);
            emit(type_to_ctype(type, curr_scope), " ", fielddef.name, ";\n");
        }

        emit("}");
    } break;
    default:
        fixme();
    }

    emit(";\n\n");
}

void Compiler::compile(fs::path file_path, bool is_main_mod) {
    fs::path filename_only = file_path.filename();
    fs::path filename_only_without_ext = filename_only;
    filename_only_without_ext.replace_extension();
    module_prefix = alloc_str(filename_only_without_ext.string() + "_");

    std::ifstream file(file_path.string());
    if (!file.is_open()) {
        err("cannot open file ", file_path.string());
    }

    std::stringstream buffer;

    buffer << file.rdbuf();
    auto program = buffer.str();
    DP(program);

    std::cout << "Compiling " << file_path << std::endl;
    auto input_file_dir = file_path.parent_path();

    Block *toplevel_block = nullptr;
    try {
        Parser parser(program, 0, file_path.string(), &ast_arena);
        toplevel_block = parser.parse();
    } catch (ParserError pe) {
        std::cout << pe.msg << std::endl;
        std::exit(-1);
    } catch (ScannerError se) {
        std::cout << se.msg << std::endl;
        std::exit(-1);
    }

    DP(toplevel_block->stmts.size());

    emit_block(toplevel_block, false);
    auto gencode = c_code.str();
    std::cout << "Emitted code: \n\n" << gencode << "\n\n";

    fs::path parent_dir = file_path.parent_path();
    auto swandir = parent_dir / ".swan";
    if (!fs::exists(swandir)) {
        fs::create_directory(swandir);
    }

    filename_only.replace_extension(".c");
    auto output_cfile_path = swandir / filename_only;

    std::cout << "Output file name: " << output_cfile_path.string() << "\n\n";
    if (fs::exists(output_cfile_path)) {
        fs::remove(output_cfile_path);
    }

    std::ofstream outc(output_cfile_path.string());
    outc << gencode;

    //  For debugging...
    //  auto fmt_cmd = "clang-format -i " + output_cfile_path.string();
    //  system(fmt_cmd.c_str());
}

} // namespace swan