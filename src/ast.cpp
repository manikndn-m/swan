#include "ast.h"

namespace swan {

void dbgprint_expr(AstNode *expr, int level)
{
    int i = level;
    while (i--) {
        print("-");
    }
    
    print(" ");

    if (expr->kind == ak_intlit) {
        print(expr->as<IntLitExpr>()->val, "\n");
    } else if (expr->kind == ak_id) {
        print(expr->as<IdExpr>()->id, ", id\n");
    } else if (expr->kind == ak_binexpr) {
        auto bin = expr->as<BinExpr>();
        print("BINOP: ", bin->op, "\n");
        dbgprint_expr(bin->left, level + 1);
        dbgprint_expr(bin->right, level + 1);
    } else {
        fixme();
    }
}

} // namespace swan
