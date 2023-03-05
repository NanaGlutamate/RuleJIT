#include <cctype>
#include <regex>
#include <set>

#include "frontend/lexer.h"

// extern "C" const int yy_nxt[][128];

// extern "C" const int yy_accept[];

extern "C" int yylex(int guidence, char *start_ptr);

namespace {

using namespace std::literals;

std::set<std::string_view> keyWords{
    "if", "else", "while", "func", "var", "type", "struct", "class", "dynamic", "extern", "return", "not",
};

} // namespace

namespace rulejit {

void ExpressionLexer::extend(Guidence guidence) {
    if (begin == end) {
        // EOF | endline
        if (type != TokenType::ENDLINE) {
            type = TokenType::ENDLINE;
        } else {
            type = TokenType::END;
        }
        return;
    }
    while (isspace(*next) && (!charEqual('\n') || guidence != Guidence::SEEK_ENDLINE)) {
        // SPACE
        next++;
    }
    begin = next;
    if (isdigit(*next)) {
        // real | int literal
        static std::regex integer(R"([1-9][0-9]*|0(?:x[0-9a-fA-F]*|b[0-1]*)?)",
                                  std::regex_constants::optimize | std::regex_constants::ECMAScript);
        static std::regex real(
            R"((?:(?:[1-9][0-9]*|0)\.[0-9]*(?:e-?[1-9][0-9]*)?)|(?:(?:[1-9][0-9]*|0)e-?[1-9][0-9]*))",
            std::regex_constants::optimize | std::regex_constants::ECMAScript);
        do {
            next++;
        } while (isdigit(*next) || isalpha(*next) || charEqual('.') || (charEqual('-') && *(next - 1) == 'e'));
        if(std::regex_match(topCopy(), real)){
            // real
            type = TokenType::REAL;
        }else if(std::regex_match(topCopy(), integer)){
            // int
            type = TokenType::INT;
        }else{
            return setError("unknow digit: "s + topCopy());
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
    } else if (charEqual('_') || isalpha(*next) || *next >= 0x80) {
        // keywords | identifier
        do {
            next++;
        } while (charEqual('_') || isalpha(*next) || isdigit(*next) || *next >= 0x80);
        auto indent = top();
        if (keyWords.find(indent) != keyWords.end()) {
            // keywords
            type = TokenType::KEYWORD;
        } else {
            // identifier
            type = TokenType::IDENT;
        }
    } else if (charEqual(';') || charEqual('\n')) {
        // endline
        type = TokenType::ENDLINE;
        next++;
        begin = next;
        auto reserve = begin;
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
        // symbol
        type = TokenType::SYM;
        static std::regex symbol(R"(.|&&|\|\||>=|<=|==|->|!=|>>|<<)",
                                 std::regex_constants::optimize | std::regex_constants::ECMAScript);
        if (std::regex_match(std::string(begin, next - begin + 1), symbol)) {
            next++;
            if (std::regex_match(std::string(begin, next - begin + 1), symbol)) {
                next++;
            }
        }
    }
}

} // namespace rulejit