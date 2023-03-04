#include <cctype>

#include "frontend/lexer.h"

// extern "C" const int yy_nxt[][128];

// extern "C" const int yy_accept[];

extern "C" int yylex(int guidence, char* start_ptr);

namespace{



}

namespace rulejit{

void ExpressionLexer::extend(Guidence guidence){
    if(begin == end || type == TokenType::END){
        type = TokenType::END;
        return;
    }
    while(charEqual(' ')||charEqual('\t')||charEqual('\r')){
        next++;
    }
    begin = next;
    if(isdigit(*begin)){
        // number literal

    }else if(charEqual('"')){
        // string literal

    }else if(*next=='_'||isalpha(*next>='a')||*next>=0x80){
        // identifier or keywords

    }
}

}