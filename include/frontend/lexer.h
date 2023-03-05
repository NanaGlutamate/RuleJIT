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
        NONE,
        START,
        SEEK_ENDLINE,
    };
    using Ele = char;
    using BufferType = std::string;
    using BufferView = std::string_view;
    ExpressionLexer() = default;
    template <typename SrcTy> ExpressionLexer& load(SrcTy &&expression) {
        buffer = std::forward<SrcTy>(expression);
        restartLexer();
        return *this;
    }
    template <typename ReceiveTy> ExpressionLexer &operator>>(ReceiveTy &dst) {
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
                pop();
            } else {
                dst = popCopy();
            }
        } else {
            std::stringstream tmp;
            tmp << pop();
            tmp >> dst;
            my_assert(tmp.eof(), std::string("cannot change literl to type ") + typeid(ReceiveTy).name());
        }
        return *this;
    }
    TokenType tokenType() { return type; }
    BufferType topCopy() { return BufferType(begin, next - begin); }
    BufferType popCopy(Guidence guidence = Guidence::NONE) {
        BufferType tmp = topCopy();
        begin = next;
        extend(guidence);
        return tmp;
    }
    BufferView top() { return BufferView(begin, next - begin); }
    BufferView pop(Guidence guidence = Guidence::NONE) {
        BufferView tmp = top();
        begin = next;
        extend(guidence);
        return tmp;
    }
    template<typename Ty, typename This> requires std::is_same_v<ExpressionLexer, std::remove_all_extents_t<This>>
    friend ExpressionLexer& operator|(Ty&&src, This&& e){
        static ExpressionLexer real_e;
        real_e = std::forward<This>(e);
        real_e.load(std::forward<Ty>(src));
        return real_e;
    }

  protected:
    char readEscape(char *&p) {
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
    void restartLexer() {
        errorHandler.err = false;
        // auto noSpaceEnd =
        //     std::remove_if(buffer.begin(), buffer.end(), [](auto c) { return c == ' ' || c == '\t' || c == '\r'; });
        // buffer.erase(noSpaceEnd, buffer.end());
        begin = buffer.c_str();
        next = begin;
        end = begin + buffer.size();
        extend(Guidence::START);
    }
    const Ele *begin;
    const Ele *next;
    const Ele *end;
    TokenType type;
    BufferType buffer;
};

// struct BufferedExpressionLexer : public ExpressionLexer {
//     void back(size_t steps = 1) {}
//     void clear(size_t depth = 0) {}

//   private:
//     struct state {
//         const char *begin, *next;
//     };
//     size_t current;
//     std::vector<state> history;
//     void restartLexer() {
//         current=0;
//         ExpressionLexer::restartLexer();
//         clear();
//     }
// };

} // namespace rulejit