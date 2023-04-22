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
#include "tools/seterror.hpp"

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

/**
 * @brief tool function to change TokenType to string
 *
 * @param t TokenType input
 * @return std::string string output
 */
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

    /// @brief error handler to handle error information and state when error
    struct Error {
        bool err;
        const char *begin, *next;
        std::string info;
        operator bool() { return err; }
        void clear() { err = false; }
    } errorHandler;

    /// @brief guidence used to control lexer behavior when pop
    enum class Guidence {
        NONE = 0,
        START = 1,
        IGNORE_BREAK = 2,
        UNARY_OP = 4,
        NO_MULTICHARSYM = 8,
    };

    using Ele = char;
    using BufferType = std::string;
    using BufferView = std::string_view;

    /**
     * @brief load string to lexer
     *
     * TOSO: add concept restriction
     *
     * @tparam SrcTy const string& or string&&, used in std::forward
     * @param expression string need to load
     * @return ExpressionLexer& reference to self
     */
    template <typename SrcTy> ExpressionLexer &load(SrcTy &&expression) {
        buffer = std::forward<SrcTy>(expression);
        restart();
        return *this;
    }

    /**
     * @brief stream operator<< to load string
     *
     * @tparam SrcTy const string& or string&&, used in std::forward
     * @param expression string need to load
     * @return ExpressionLexer& reference to self
     */
    template <typename SrcTy> ExpressionLexer &operator<<(SrcTy &&expression) {
        buffer = std::forward<SrcTy>(expression);
        restart();
        return *this;
    }

    /**
     * @brief stream operator>> to pop value
     *
     * @tparam ReceiveTy type of target value
     * @param dst variable to receive value
     * @return ExpressionLexer& reference to self
     */
    template <typename ReceiveTy> ExpressionLexer &operator>>(ReceiveTy &dst) { return fill(dst, Guidence::NONE); }

    /**
     * @brief function to pop value
     * @attention will tranfer escape character if type of token and receiver is both string,
     * else use std::stringstream to convert
     *
     * @tparam ReceiveTy type of target value
     * @param dst variable to receive value
     * @param guidence ExpressionLexer::Guidence to control lexer behavior when pop
     * @return ExpressionLexer& ExpressionLexer& reference to self
     */
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
            if (!tmp.eof()) {
                error(std::string("cannot change literl to type ") + typeid(ReceiveTy).name());
            };
        }
        return *this;
    }

    /**
     * @brief get token type of current token
     *
     * @return TokenType token type
     */
    TokenType tokenType() { return type; }

    /**
     * @brief get current token as std::string without pop it
     *
     * @return BufferType current token
     */
    BufferType topCopy() { return BufferType(begin, next - begin); }

    /**
     * @brief pop current token and return it as std::string
     *
     * @param guidence guidence to control lexer behavior when pop
     * @return BufferType current token
     */
    BufferType popCopy(Guidence guidence = Guidence::NONE) {
        BufferType tmp = topCopy();
        begin = pre = next;
        extend(guidence);
        return tmp;
    }

    /**
     * @brief get current token as std::string_view without pop it
     *
     * @return BufferView current token
     */
    BufferView top() { return BufferView(begin, next - begin); }

    /**
     * @brief pop current token and return it as std::string_view
     *
     * @param guidence guidence to control lexer behavior when pop
     * @return BufferView current token
     */
    BufferView pop(Guidence guidence = Guidence::NONE) {
        BufferView tmp = top();
        begin = pre = next;
        extend(guidence);
        return tmp;
    }

    /**
     * @brief get first char of current token without pop it
     *
     * @return char first char
     */
    char topChar() { return *begin; }

    /**
     * @brief pipe operator| to load string
     *
     * @tparam Ty const string& or string&&, used in std::forward
     * @param src source string
     * @param e receiver ExpressionLexer
     * @return ExpressionLexer& reference to receiver
     */
    template <typename Ty> friend ExpressionLexer &operator|(Ty &&src, ExpressionLexer &e) {
        e.load(std::forward<Ty>(src));
        return e;
    }

    /**
     * @brief get current state of ExpressionLexer, used in loadState
     *
     * @see ExpressionLexer::loadState
     *
     * @return std::tuple<const char *, const char *, bool, rulejit::TokenType>
     */
    std::tuple<const char *, const char *, bool, rulejit::TokenType> getState() {
        return std::make_tuple(begin, next, errorHandler.err, type);
    }

    /**
     * @brief load stored state
     * @attention state must generated by the same object, and no load operation between
     * get and load of state
     *
     * @param s state need to load
     */
    void loadState(const std::tuple<const char *, const char *, bool, rulejit::TokenType> &s) {
        std::tie(begin, next, errorHandler.err, type) = s;
    }

    /**
     * @brief foresee next depth-th token
     *
     * @param depth number of token to foresee
     * @return std::tuple<TokenType, std::string_view> foreseed token type and value
     */
    std::tuple<TokenType, std::string_view> foresee(size_t depth = 1) {
        if (depth == 0) {
            return std::make_tuple(type, top());
        }
        auto s = getState();
        while (depth--) {
            pop();
        }
        auto t = std::make_tuple(type, top());
        loadState(s);
        return t;
    }

    /**
     * @brief restart lexer, clear all state but reserve buffer
     *
     */
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

    /**
     * @brief get index in buffered string of first char of current token
     *
     * @return size_t
     */
    size_t beginIndex() const { return begin - buffer.data(); }

    /**
     * @brief pointer to start position in buffered string of each new line,
     * used to generate better error message
     *
     */
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
        error(info);
    }

    bool charEqual(Ele e) { return *next == e; }

    void extend(Guidence guidence);

    const Ele *pre;
    const Ele *begin;
    const Ele *next;
    const Ele *end;
    TokenType type;
    BufferType buffer;
};

} // namespace rulejit