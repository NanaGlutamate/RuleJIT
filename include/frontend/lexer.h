#pragma once

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <type_traits>

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
};

#undef TOKEN

#define TOKEN(t) { TokenType::t, #t }

std::ostream &operator<<(std::ostream &o, TokenType type) {
    return o << std::map<TokenType, std::string>{
               TOKEN(INT), TOKEN(REAL),    TOKEN(STRING),  TOKEN(IDENT),
               TOKEN(SYM), TOKEN(ENDLINE), TOKEN(KEYWORD), TOKEN(END),
           }[type];
}

#undef TOKEN

class ExpressionLexer {
  public:
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
        dst = pop();
        return *this;
    }
    TokenType tokenType() { return type; }
    BufferView top() { return BufferView(begin, next - begin); }
    BufferView pop(Guidence guidence = Guidence::None) {
        BufferView tmp = top();
        begin = next;
        extend(guidence);
        return tmp;
    }

  private:
    bool charEqual(Ele e) { return *next == e; }
    void extend(Guidence guidence);
    void restartLexer() {
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