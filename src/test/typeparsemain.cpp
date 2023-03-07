#include <iostream>

#include "frontend/lexer.h"
#include "ast/type.hpp"

void test(const std::string& s){
    using namespace rulejit;
    static ExpressionLexer lexer;
    auto type = s | lexer | TypeParser();
    if(type.isValid() && type.toString()==s){
        std::cout << "test pass: " << type.toString() << std::endl;
    }else{
        std::cout << "error" << std::endl;
    }
}

int main(){
    test("func(int,int)int");
}