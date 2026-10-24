#include "scanner.h"

#include "diag.h"
#include <cassert>
#include <functional>
#include <unordered_map>

namespace swan {

void Scanner::take_while(std::string &acc, std::function<bool(char)> pred)
{
    acc.clear();
    while (pred(*it)) {
        acc += *it;
        advance();
        if (eof()) {
            break;
        }
    }
}

void Scanner::next(Token &out)
{
    auto is_keyword = [&](const std::string &s, TokenKind &out) {
        auto key_it = keywordmap.find(s);
        if (key_it == keywordmap.end()) {
            return false;
        }

        out = key_it->second;
        return true;
    };

    auto scan_escape = [&](char endc) {
        assert(*it == '\\');
        advance();

        if (eof() || *it == endc) {
            err(curpos(), "unterminated escape sequence");
        }

        switch (*it) {
        case 'r':
            advance();
            return '\r';
        case 'n':
            advance();
            return '\n';
        case 't':
            advance();
            return '\t';
        case 'b':
            advance();
            return '\b';
        default:
            err(curpos(), "unrecognized escape sequence");
        }

        return char(0);
    };

    if (eof()) {
        out.kind = tok_eof;
        return;
    }

    while (!eof() && is_whitespace(*it)) {
        advance();
    }

    if (eof()) {
        out.kind = tok_eof;
        return;
    }

    out.srange.start = pos;

    if (*it == '\r') {
        err(curpos(),
            "carriage return characters are not supported, use only line feed "
            "characters for newlines");
    }

    bool is_lb = false;
    while (*it == '\n') {
        if (*it == '\n') {
            it++;
        }

        if (eof()) {
            out.kind = tok_eof;
            return;
        }

        pos.col = 1;
        pos.line++;
        is_lb = true;
    }

    if (!eof() && (*it == '/')) {
        advance();
        if (!eof() && *it == '/') {
            while (!eof() && *it != '\n') {
                advance();
            }
            pos.col = 1;
            pos.line++;
            is_lb = true;
        } else {
            err(curpos(), "unrecognized character '/' (ASCII code 47)");
        }
    }

    if (is_lb) {
        out.kind = tok_linebreak;
    } else if (std::isalpha(*it) || *it == '_') {
        take_while(accum, [](char c) { return std::isalnum(c) || c == '_'; });

        TokenKind keyword;
        if (is_keyword(accum, keyword)) {
            out.kind = keyword;
        } else {
            out.kind = tok_id;
            out.idval = accum;
        }
    } else if (std::isdigit(*it)) {
        take_while(accum, [](char c) {
            return std::isdigit(c) || c == '.' || c == 'x' || c == 'o';
        });

        bool is_floating = false;
        if (accum.find('.') != std::string::npos) {
            is_floating = true;
        }

        if (is_floating) {
            out.kind = tok_floatlit;
            out.fval = std::stod(accum);
        } else {
            out.kind = tok_intlit;
            out.ival = std::stoi(accum);
        }

        if (!eof() && (*it == 'f' || *it == 'i')) {
        }
    } else if (*it == '"') {
        accum.clear();
        advance(); // Skip the '"'
        while (*it != '"') {
            if (*it == '\r' || *it == '\n') {
                pos.line++;
                pos.col = 1;
                it++;
            }

            // Handle escape sequence...
            if (*it == '\\') {
                char c = scan_escape('"');
                accum += c;
            } else {
                accum += *it;
                advance();
            }
            if (eof()) {
                break;
            }
        }
        advance(); // Skip the '"'
        out.kind = tok_strlit;
        out.strval = accum;
    } else if (*it == '\'') {
        advance();

        if (eof()) {
            err(curpos(), "unterminated character literal");
        }

        uint64_t ch;
        if (*it == '\\') {
            char c = scan_escape('"');
            ch = c;
        } else {
            ch = *it;
            advance();
        }

        if (eof() || *it != '\'') {
            err(curpos(), "unterminated character literal");
        }

        advance();
        out.kind = tok_charlit;
        out.charval = ch;
    } else {
// Macro SCO handles single character operator/delimiter.
#define SCO(C, TOK)                                                            \
    {                                                                          \
    case C:                                                                    \
        advance();                                                             \
        out.kind = TOK;                                                        \
        break;                                                                 \
    }
        switch (*it) {
        case '+':
            advance();
            if (!eof() && *it == '+') {
                advance();
                out.kind = tok_inc;
                break;
            } else if (!eof() && *it == '=') {
                advance();
                out.kind = tok_pluseq;
                break;
            }
            out.kind = tok_plus;
            break;
        case '-':
            advance();
            if (!eof() && *it == '-') {
                advance();
                out.kind = tok_dec;
                break;
            } else if (!eof() && *it == '=') {
                advance();
                out.kind = tok_minuseq;
                break;
            }
            out.kind = tok_plus;
            break;

        case '>':
            advance();
            if (!eof() && *it == '=') {
                advance();
                out.kind = tok_gteq;
                break;
            } else if (!eof() && *it == '>') {
                advance();
                out.kind = tok_rshift;
                break;
            }
            out.kind = tok_gt;
            break;

        case '<':
            advance();
            if (!eof() && *it == '=') {
                advance();
                out.kind = tok_lteq;
                break;
            } else if (!eof() && *it == '<') {
                advance();
                out.kind = tok_lshift;
                break;
            }
            out.kind = tok_lt;
            break;

        case '&':
            advance();
            if (!eof() && *it == '&') {
                advance();
                out.kind = tok_and;
                break;
            }
            out.kind = tok_btand;
            break;

        case '|':
            advance();
            if (!eof() && *it == '|') {
                advance();
                out.kind = tok_or;
                break;
            }
            out.kind = tok_btor;
            break;

        case '!':
            advance();
            if (!eof() && *it == '=') {
                advance();
                out.kind = tok_nequals;
                break;
            }
            out.kind = tok_not;
            break;

        case '=':
            advance();
            if (!eof() && *it == '=') {
                advance();
                out.kind = tok_equals;
                break;
            }
            out.kind = tok_assign;
            break;

            SCO('*', tok_mul)
            SCO('/', tok_div)
            SCO('^', tok_btxor)
            SCO('(', tok_oparen)
            SCO(')', tok_cparen)
            SCO(',', tok_comma)
            SCO('{', tok_obrace)
            SCO('}', tok_cbrace)
            SCO(';', tok_semic)
            SCO('.', tok_dot)
            SCO(':', tok_colon)

        default:
            err(curpos(), std::string("unrecognized character '") + *it +
                              "' (ASCII code " + std::to_string(int(*it)) +
                              ")");
        }
    }

    out.srange.end = pos;
} // namespace swan

} // namespace swan