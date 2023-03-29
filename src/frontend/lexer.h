/**
 * @file lexer.h
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
#pragma once

#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#include "tools/myassert.hpp"

namespace rulejit {

#define TOKEN(t) t

/// @brief token type
enum class TokenType {
    TOKEN(INT),
    TOKEN(REAL),
    TOKEN(STRING),
    TOKEN(IDENT),
    TOKEN(SYM), // symbol and keywords
    TOKEN(ENDLINE),
    TOKEN(END),
    TOKEN(UNKNOWN),
};

#undef TOKEN

#define TOKEN(t)                                                                                                       \
    { TokenType::t, #t }

/// @brief tool function to change TokenType to string
/// @param t TokenType input
/// @return string output
inline std::string to_string(TokenType t) {
    static const std::map<TokenType, std::string> table{
        TOKEN(INT), TOKEN(REAL), TOKEN(STRING), TOKEN(IDENT), TOKEN(SYM), TOKEN(ENDLINE), TOKEN(END), TOKEN(UNKNOWN),
    };
    return table.find(t)->second;
}

inline std::ostream &operator<<(std::ostream &o, TokenType type) { return o << to_string(type); }

#undef TOKEN

/**
 * @brief main class of lexer
 * 
 */
struct ExpressionLexer {    
    ExpressionLexer() = default;
    ExpressionLexer(const ExpressionLexer &e) = delete;
    ExpressionLexer(ExpressionLexer &&e) = delete;
    ExpressionLexer &operator=(const ExpressionLexer &e) = delete;
    ExpressionLexer &operator=(ExpressionLexer &&e) = delete;

    struct Error {
        bool err;
        const char *begin, *next;
        std::string info;
        operator bool() { return err; }
        void clear() { err = false; }
    } errorHandler;
    enum class Guidence {
        NONE,
        START,
        IGNORE_BREAK,
    };
    using Ele = char;
    using BufferType = std::string;
    using BufferView = std::string_view;
    template <typename SrcTy> ExpressionLexer &load(SrcTy &&expression) {
        buffer = std::forward<SrcTy>(expression);
        restart();
        return *this;
    }
    template <typename SrcTy> ExpressionLexer &operator<<(SrcTy &&expression) {
        buffer = std::forward<SrcTy>(expression);
        restart();
        return *this;
    }
    template <typename ReceiveTy> ExpressionLexer &operator>>(ReceiveTy &dst) { return fill(dst, Guidence::NONE); }
    template <typename ReceiveTy> ExpressionLexer &fill(ReceiveTy &dst, Guidence guidence) {
        if constexpr (std::is_same_v<ReceiveTy, std::string>) {
            if (type == TokenType::STRING) {
                std::string dst_tmp;
                for (auto p = begin + 1; p != next - 1; ++p) {
                    if (*p == '\\') {
                        dst_tmp.push_back(readEscape(++p));
                    } else {
                        dst_tmp.push_back(*p);
                    }
                }
                dst = std::move(dst_tmp);
                pop(guidence);
            } else {
                dst = popCopy(guidence);
            }
        } else {
            std::stringstream tmp;
            tmp << pop(guidence);
            tmp >> dst;
            my_assert(tmp.eof(), std::string("cannot change literl to type ") + typeid(ReceiveTy).name());
        }
        return *this;
    }
    TokenType tokenType() { return type; }
    // void reExtend(Guidence guidence = Guidence::NONE) {
    //     begin = next = pre;
    //     extend(guidence);
    // }
    BufferType topCopy() { return BufferType(begin, next - begin); }
    BufferType popCopy(Guidence guidence = Guidence::NONE) {
        BufferType tmp = topCopy();
        begin = pre = next;
        extend(guidence);
        return tmp;
    }
    BufferView top() { return BufferView(begin, next - begin); }
    char topChar() {
        return *begin;
    }
    BufferView pop(Guidence guidence = Guidence::NONE) {
        BufferView tmp = top();
        begin = pre = next;
        extend(guidence);
        return tmp;
    }
    template <typename Ty> friend ExpressionLexer &operator|(Ty &&src, ExpressionLexer &e) {
        e.load(std::forward<Ty>(src));
        return e;
    }
    std::tuple<const char *, const char *, bool, rulejit::TokenType> getState() {
        return std::make_tuple(begin, next, errorHandler.err, type);
    }
    void loadState(const std::tuple<const char *, const char *, bool, rulejit::TokenType> &s) {
        std::tie(begin, next, errorHandler.err, type) = s;
    }
    std::tuple<TokenType, std::string_view> foresee(size_t depth = 1) {
        auto s = getState();
        pop();
        auto t = (depth == 1) ? std::make_tuple(type, top()) : foresee(depth - 1);
        loadState(s);
        return t;
    }
    void restart() {
        linePointer.clear();
        linePointer.push_back(buffer.data());
        errorHandler.clear();
        begin = buffer.data();
        pre = next = begin;
        end = begin + buffer.size();
        type = TokenType::ENDLINE;
        extend(Guidence::START);
    }
    size_t beginIndex() const { return begin - buffer.data(); }
    std::vector<const char *> linePointer;

  private:
    char readEscape(const char *&p) {
        static const std::map<char, char> escape{
            {'n', '\n'},
            {'0', '\0'},
            {'r', '\r'},
            {'t', '\t'},
        };
        if (*p == 'x') {
            char tmp = hex(*(++p));
            tmp *= 0x10;
            tmp += hex(*(++p));
            return tmp;
        } else if (auto it = escape.find(*p); it != escape.end()) {
            return it->second;
        } else {
            return *p;
        }
    }
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
    const Ele *pre;
    const Ele *begin;
    const Ele *next;
    const Ele *end;
    TokenType type;
    BufferType buffer;
    // std::istream *stream;
};

} // namespace rulejit