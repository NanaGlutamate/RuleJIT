#pragma once

#include <type_traits>
#include <string>
#include <iostream>
#include <map>


namespace rulejit {

#define TOKEN(t) t
enum class TokenType{
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
#define TOKEN(t) {TokenType::t, #t}
std::ostream& operator<<(std::ostream& o, TokenType type){
    return o << std::map<TokenType, std::string>{
        TOKEN(INT),
        TOKEN(REAL),
        TOKEN(STRING),
        TOKEN(IDENT),
        TOKEN(SYM),
        TOKEN(ENDLINE),
        TOKEN(KEYWORD),
        TOKEN(END),
    }[type];
}
#undef TOKEN

class ExpressionLexer {
public:
    using Ele = char;
    using BufferType = std::string;
    using BufferView = std::string_view;
    // static ExpressionLexer& getLexer(){static ExpressionLexer e; return e;}
    ExpressionLexer() = default;
    template<typename SrcTy>
    void loadExpression(SrcTy&& expression){buffer = std::forward<SrcTy>(expression); restartLexer();}
    template<typename ReceiveTy>
    ExpressionLexer& operator>>(ReceiveTy& dst){dst = pop(); return *this;}
    TokenType tokenType(){return type;}
    BufferView top(){return BufferView(begin, next - begin);}
    BufferView pop(){BufferView tmp = top(); begin = next; extend(); return tmp;}
private:
    bool charEqual(Ele e){return *next == e;}
    void extend();
    void restartLexer(){begin = buffer.c_str(); next = begin; end = begin + buffer.size(); extend();}
    const Ele* begin;
    const Ele* next;
    const Ele* end;
    TokenType type;
    BufferType buffer;
};

} // namespace rulejit