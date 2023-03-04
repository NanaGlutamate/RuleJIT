#pragma once

#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <type_traits>

#include "tools/myassert.hpp"

namespace rulejit {

#define TOKEN(t) t

enum class TokenType {
    TOKEN(INT),
    TOKEN(REAL),
    TOKEN(STRING),
    TOKEN(IDENT),
    TOKEN(SYM),
    TOKEN(ENDLINE),
    TOKEN(KEYWORD),
    TOKEN(END),
    TOKEN(UNKNOWN),
};

#undef TOKEN

#define TOKEN(t)                                                                                                       \
    { TokenType::t, #t }

inline std::ostream &operator<<(std::ostream &o, TokenType type) {
    return o << std::map<TokenType, std::string>{
               TOKEN(INT),     TOKEN(REAL),    TOKEN(STRING), TOKEN(IDENT),   TOKEN(SYM),
               TOKEN(ENDLINE), TOKEN(KEYWORD), TOKEN(END),    TOKEN(UNKNOWN),
           }[type];
}

#undef TOKEN

class ExpressionLexer {
  public:
    struct Error {
        bool err;
        const char *begin, *next;
        std::string info;
        operator bool() { return err; }
    } errorHandler;
    enum class Guidence {
        None,
        Start,
    };
    using Ele = char;
    using BufferType = std::string;
    using BufferView = std::string_view;
    // static ExpressionLexer& getLexer(){static ExpressionLexer e; return e;}
    ExpressionLexer() = default;
    template <typename SrcTy> void loadExpression(SrcTy &&expression) {
        buffer = std::forward<SrcTy>(expression);
        restartLexer();
    }
    template <typename ReceiveTy> ExpressionLexer &operator>>(ReceiveTy &dst) {
        if constexpr (std::is_same_v<ReceiveTy, std::string>) {
            my_assert(type == TokenType::STRING);
            std::string dst_tmp;
            for (auto p = begin + 1; p != next - 1; ++p) {
                if (*p == '\\') {
                    p++;
                    if (*p == 'x') {
                        char tmp = hex(*(++p));
                        tmp *= 0x10;
                        tmp += hex(*(++p));
                        dst_tmp.push_back(tmp);
                    } else if (*p == 'n') {
                        dst_tmp.push_back('\n');
                    } else if (*p == '0') {
                        dst_tmp.push_back('\0');
                    } else if (*p == 'r') {
                        dst_tmp.push_back('\r');
                    } else if (*p == 't') {
                        dst_tmp.push_back('\t');
                    } else {
                        dst_tmp.push_back(*p);
                    }
                } else {
                    dst_tmp.push_back(*p);
                }
            }
            dst = std::move(dst_tmp);
            pop();
        } else {
            std::stringstream tmp;
            tmp << pop();
            tmp >> dst;
        }
        return *this;
    }
    TokenType tokenType() { return type; }
    BufferType topCopy() { return BufferType(begin, next - begin); }
    BufferType popCopy(Guidence guidence = Guidence::None) {
        BufferType tmp = topCopy();
        begin = next;
        extend(guidence);
        return tmp;
    }
    BufferView top() { return BufferView(begin, next - begin); }
    BufferView pop(Guidence guidence = Guidence::None) {
        BufferView tmp = top();
        begin = next;
        extend(guidence);
        return tmp;
    }

  private:
    int hex(char c) {
        if (c >= '0' && c <= '9') {
            return c - '0';
        } else if (c >= 'a' && c <= 'f') {
            return c - 'a' + 0xa;
        } else if (c >= 'A' && c <= 'F') {
            return c - 'A' + 0xa;
        }
        return -1;
    }
    void setError(const std::string &info) {
        type = TokenType::UNKNOWN;
        errorHandler = {
            true,
            begin,
            end,
            info,
        };
    }
    bool charEqual(Ele e) { return *next == e; }
    void extend(Guidence guidence);
    void restartLexer() {
        errorHandler.err = false;
        // auto noSpaceEnd =
        //     std::remove_if(buffer.begin(), buffer.end(), [](auto c) { return c == ' ' || c == '\t' || c == '\r'; });
        // buffer.erase(noSpaceEnd, buffer.end());
        begin = buffer.c_str();
        next = begin;
        end = begin + buffer.size();
        extend(Guidence::Start);
    }
    const Ele *begin;
    const Ele *next;
    const Ele *end;
    TokenType type;
    BufferType buffer;
};

} // namespace rulejit