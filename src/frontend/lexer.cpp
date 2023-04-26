/**
 * @file lexer.cpp
 * @author djw
 * @brief FrontEnd/Lexer
 * @date 2023-03-28
 *
 * @details Lexer
 *
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-28</td><td>Initial version.</td></tr>
 * </table>
 */
#include <cctype>
#include <regex>
#include <set>

#include "defines/language.hpp"
#include "frontend/lexer.h"

// extern "C" const int yy_nxt[][128];

// extern "C" const int yy_accept[];

// extern "C" int yylex(int guidence, char *start_ptr);

namespace {

using namespace std::literals;

} // namespace

namespace rulejit {

void ExpressionLexer::extend(Guidence guidence) {
    while (isspace(*next)) {
        // SPACE
        if (charEqual('\n')) {
            linePointer.push_back(next);
        }
        if (!config::ignoreAllBreak && guidence != Guidence::IGNORE_BREAK && charEqual('\n')) {
            type = TokenType::ENDLINE;
            next++;
            return;
        }
        next++;
    }
    begin = next;
    if (begin == end) {
        // EOF | endline
        if (type != TokenType::ENDLINE) {
            type = TokenType::ENDLINE;
        } else {
            type = TokenType::END;
        }
        return;
    }
    if (charEqual('_') || isalpha(*next) || *next < 0) {
        // keywords | identifier
        do {
            next++;
        } while (charEqual('_') || isalpha(*next) || isdigit(*next) || *next < 0);
        auto indent = top();
        if (KEYWORDS.find(indent) != KEYWORDS.end()) {
            // keywords
            type = TokenType::SYM;
        } else {
            // identifier
            type = TokenType::IDENT;
        }
    } else if (isdigit(*next)) {
        // real | int literal
        static const std::regex integer(R"([1-9][0-9]*|0(?:x[0-9a-fA-F]*|b[0-1]*)?)",
                                        std::regex_constants::optimize | std::regex_constants::ECMAScript);
        static const std::regex real(
            R"((?:(?:[1-9][0-9]*|0)\.[0-9]*(?:e-?[1-9][0-9]*)?)|(?:(?:[1-9][0-9]*|0)e-?[1-9][0-9]*))",
            std::regex_constants::optimize | std::regex_constants::ECMAScript);
        do {
            next++;
        } while (isdigit(*next) || isalpha(*next) || charEqual('.') || (charEqual('-') && *(next - 1) == 'e'));
        if (std::regex_match(topCopy(), real)) {
            // real
            type = TokenType::REAL;
        } else if (std::regex_match(topCopy(), integer)) {
            // int
            type = TokenType::INT;
        } else {
            return setError("expect digit literal, found: "s + topCopy());
        }
    } else if (charEqual('"')) {
        // string literal
        type = TokenType::STRING;
        do {
            if (charEqual('\\')) {
                next++;
                if (charEqual('x')) {
                    if (next++; hex(*next) != -1) {
                        if (next++; hex(*next) != -1) {
                            continue;
                        }
                    }
                    return setError("expect digit in \"\\x\" escape character, found: '"s + *next + "'"s);
                }
            } else if (charEqual('\0') || charEqual('\n')) {
                return setError("string literal not end"s);
            }
            next++;
        } while (!charEqual('"'));
        next++;
    } else if (charEqual(';')) {
        // endline
        // linePointer.push_back(next);
        type = TokenType::ENDLINE;
        next++;
        begin = next;
        auto reserve = begin;
        // eat multi endline
        if (extend(guidence); type != TokenType::ENDLINE) {
            type = TokenType::ENDLINE;
            errorHandler.err = false;
            begin = reserve - 1;
            next = reserve;
        }
    } else {
        // comment | symbol
        if (charEqual('/') && next + 1 != end && *(next + 1) == '/') {
            // comment
            next += 2;
            while (next != end && *next != '\n') {
                next++;
            }
            begin = next;
            return extend(guidence);
        }
        if (charEqual('`')) {
            // TODO: tokenized symbol? like `+(a, b) // return a+b
            // TODO: symbolized identifier?
        }
        // symbol
        type = TokenType::SYM;
        next++;
        while (!(static_cast<int>(guidence) & static_cast<int>(Guidence::NO_MULTICHARSYM)) &&
               BUILD_IN_MULTICHAR_SYMBOL.contains(std::string_view(begin, next - begin + 1))) {
            next++;
        }
        // eat ENDLINE
        if (top() == "\\") {
            auto store = begin;
            if (charEqual('\r')) {
                next++;
            }
            if (charEqual('\n')) {
                next++;
                begin = next;
                return extend(guidence);
            } else {
                return setError("expect '\\n' after '\\', found: "s + *next);
            }
        }
    }
}

} // namespace rulejit