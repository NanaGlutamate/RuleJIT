#include <iostream>

#include "ast/type.hpp"
#include "frontend/lexer.h"

void test(const std::string &s, const std::string &target = "") {
    using namespace rulejit;
    static ExpressionLexer lexer;
    try {
        rulejit::TypeInfo type;
        try {
            type = s | lexer | TypeParser();
        } catch (const std::exception &e) {
            std::cout << "error $1: " << e.what() << std::endl;
            return;
        }
        std::string t = (target == "") ? type.toString() : (target | lexer | TypeParser()).toString();
        if (type.isValid() && t == type.toString()) {
            std::cout << "test pass: " << type.toString() << std::endl;
        } else {
            std::cout << "test failed: " << type.toString() << " != " << t << std::endl;
        }
    } catch (const std::exception &e) {
        std::cout << "error $2: " << e.what() << std::endl;
    }
}

int main() {
    test("func(int,int):int", "func\n(int\n,\nint):\nint");
    test("func(int,int)", "func  ( int, int    )  ");
    test("func():i32");

    test("struct{a i32;b i32}", "struct\n{a i32;b i32;}");
    test("struct{a i32;b i32}", "struct{\na \ni32\nb i32;}");

    test("[]**[]i32", "[  ]* * [  ]i32");

    test("[]*struct{a int;b int}", "[]*struct{a int;b int;}");
}