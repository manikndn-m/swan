#include "parser.h"
#include "ast.h"

namespace swan {

AstNode *Parser::parse_stmt()
{
    return nullptr;
}

Block *Parser::parse_block(TokenKind endtok)
{
    Block *block = allocnode<Block>();
    std::vector<AstNode *> stmts;
    while (tok.kind != endtok) {
        auto stmt = parse_stmt();
        stmts.push_back(stmt);
    }

    return block;
}

Block *Parser::parse()
{
    // Read the first token into 'tok'.
    next();
    return parse_block(tok_eof);
}

} // namespace swan
