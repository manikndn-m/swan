#include "token.h"

namespace swan {

std::string Token::to_string() const
{
    switch (kind) {
    case tok_id:
        return "identifier '" + idval + "'";
    case tok_intlit:
        return "integer literal '" + std::to_string(ival) + "'";
    case tok_floatlit:
        return "floating-point literal '" + std::to_string(fval) + "'";
    case tok_strlit:
        return "string literal '" + strval + "'";
    case tok_linebreak:
        return "token <newline>";

#define OPD(X, Y)                                                              \
    case X:                                                                    \
        return "operator/delimiter '" + std::string(Y) + "' (" + #X + ")";
#include "opdelims.inc"
#undef OPD

#define K(X, Y)                                                                \
    case X:                                                                    \
        return "keyword '" + std::string(Y) + "'";
#include "keywords.inc"
#undef K

    default:
        return "<fixme>";
    }
}

} // namespace swan