#include <cctype>

#include "frontend/lexer.h"

namespace{



}

namespace rulejit{

void ExpressionLexer::extend(){
    if(begin == end || type == TokenType::END){
        type = TokenType::END;
        return;
    }
    while(charEqual(' ') || charEqual('\t'))next++;
    begin = next;
    if(isalpha(*begin) || begin){}
}

}