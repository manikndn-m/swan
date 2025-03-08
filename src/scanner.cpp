#include "scanner.h"
#include <cassert>
#include <functional>
#include <unordered_map>

namespace swan {

void Scanner::next(Token &out)
{
    auto scan_escseq = [&](char endc) {
        assert(*it == '\\');
        advance();

        if (eof() || *it == endc) {
            err("unterminated escape sequence");
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
        default:
            err("unrecognized escape sequence");
        }

        return char(0);
    };

    if (eof()) {
        out.kind = tok_eof;
        return;
    }

    while (is_whitespace(*it)) {
        advance();
    }

    if (eof()) {
        out.kind = tok_eof;
        return;
    }

    out.start_pos = pos;

    bool is_lb = false;
    while (*it == '\n' || (*it == '\r' && it + 1 != end && *(it + 1) == '\n')) {
        if (*it == '\r') {
            if (*(it + 1) != '\n') {
                err("stray '\n'");
            }
            it += 2;
        } else if (*it == '\n') {
            it++;
        }
        pos.col = 1;
        pos.line++;
        is_lb = true;
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
    } else if (std::isdigit(*it) || *it == '.') {
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

        if (!eof() && *it == 'f' || *it == 'i') {
        }
    } else if (*it == '"') {
        accum.clear();
        it++; // Skip the '"'
        while (*it != '"') {
            if (*it == '\r' || *it == '\n') {
                pos.line++;
                pos.col = 1;
                it++;
            }

            // Handle escape sequence...
            if (*it == '\\') {
                char c = scan_escseq('"');
                accum += c;
            } else {
                accum += *it;
                advance();
            }
            if (eof()) {
                break;
            }
        }
        it++; // Skip the '"'
        out.kind = tok_strlit;
        out.strval = accum;
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
            SCO('!', tok_not)
            SCO('(', tok_oparen)
            SCO(')', tok_cparen)
            SCO('{', tok_obrace)
            SCO('}', tok_cbrace)
            SCO(';', tok_semic)
            SCO(':', tok_colon)
        default:
            err(std::string("unrecognized character '") + *it + "' (code " +
                std::to_string(int(*it)) + ")");
        }
    }

    out.end_pos = pos;
} // namespace swan

} // namespace swan