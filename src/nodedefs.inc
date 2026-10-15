
N(BinExpr, ak_binexpr)
F(AstNode *, left)
F(AstNode *, right)
F(BinOp, op)
NEND

N(UnaryExpr, ak_unary)
F(AstNode *, operand)
F(UnaryOp, op)
NEND

N(IntLitExpr, ak_intlit)
F(uint64_t, val)
F(nlsuffix, suffix)
NEND

N(FloatLitExpr, ak_floatlit)
F(uint64_t, val)
NEND

N(StrLitExpr, ak_strlit)
F(StringRef, val)
NEND

N(IdExpr, ak_id)
F(StringRef, id)
NEND

N(FuncCallExpr, ak_funccall)
F(AstNode *, callee)
F(Span<AstNode *>, args)
NEND

N(ImportStmt, ak_import)
F(Path, path)
F(bool, as_specified)
F(StringRef, alias)
NEND

N(FieldDef, ak_fielddef)
F(TypeAnnot *, type_ann)
F(StringRef, name)
NEND

N(StructDefStmt, ak_structdef)
F(StringRef, name)
F(Span<FieldDef>, fields)
F(bool, is_object)
NEND

N(ListLitExpr, ak_listlit)
F(Span<AstNode *>, vals)
NEND

N(VarDef, ak_vardef)
F(TypeAnnot *, type_ann)
F(StringRef, id)
F(AstNode *, init)
NEND

N(VarDefStmt, ak_vardefstmt)
F(Span<VarDef>, vardefs)
NEND

N(ParamDef, ak_paramdef)
F(TypeAnnot *, type_ann)
F(StringRef, name)
NEND

N(FunctionSignatureDef, ak_funcsig)
F(Span<ParamDef>, params)
F(TypeAnnot *, return_type_ann)
NEND

N(FunctionProtoDef, ak_funcproto)
F(bool, has_receiver)
F(bool, is_good)
F(TypeAnnot *, receiver)
F(StringRef, func_name)
F(FunctionSignatureDef, sign)
NEND
