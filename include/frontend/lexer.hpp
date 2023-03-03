#pragma once

#include <type_traits>
#include <string>
#include <cctype>

namespace rulejit {

enum class TokenType{
    INT,
    REAL,
    STRING,
    
    IDENT,
    SYM,
    ENDLINE,
    KEYWORD,
    END,
};

template <typename Ele> class ExpressionLexer {
public:
    using BufferType = basic_string<Ele, char_traits<Ele>, allocator<Ele>>;
    using BufferView = basic_string_view<Ele>;
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
    void extend(){
        if(begin == end || type == TokenType::END){
            type = TokenType::END;
            return;
        }
        while(charEqual(' ') || charEqual('\t'))next++;
        begin = next;
        if(isalpha(*begin) || begin)
    }
    void restartLexer(){begin = buffer.c_str(); next = begin; end = begin + buffer.size(); extend();}
    Ele* begin;
    Ele* next;
    Ele* end;
    TokenType type;
    BufferType buffer;
};

} // namespace rulejit