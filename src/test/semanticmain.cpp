#include "ast/astprinter.hpp"
#include "frontend/parser.h"
#include "frontend/semantic.hpp"

// #define CATCH_EXCEPTION

int testCase(const std::string &s, bool expectError = false) {
    using namespace std;
    using namespace rulejit;

    static ExpressionLexer lexer;
    static ExpressionParser parser;
    static Semantic semantic;
    static ContextStack stack;
    static ASTPrinter printer;

    semantic.loadContext(stack);

#ifdef CATCH_EXCEPTION
    try {
#endif
        stack.clear();
        std::vector<std::unique_ptr<ExprAST>> tmp;
        s | lexer;
        while (lexer.tokenType() == TokenType::ENDLINE) {
            lexer.pop(ExpressionLexer::Guidence::IGNORE_BREAK);
        }
        while (lexer.tokenType() != TokenType::END) {
            tmp.push_back(lexer | parser);
            while (lexer.tokenType() == TokenType::ENDLINE) {
                lexer.pop(ExpressionLexer::Guidence::IGNORE_BREAK);
            }
        }
        tmp = std::move(tmp) | semantic;
        cout << "PASSED" << endl;
        for (auto &e : tmp) {
            cout << "    " << e->type->toString() << endl;
        }
        for (auto &f : stack.global.realFuncDefinition) {
            cout << "    " << f.first << ": " << f.second->funcType->toString() << endl;
        }
#ifdef CATCH_EXCEPTION
    } catch (std::logic_error &e) {
        if (expectError) {
            // cout << "PASSED ";
        }
        cout << "ERROR: " << e.what() << endl;
    }
#endif

    return 0;
}

int main() {
    using namespace std;
    using namespace rulejit;

    // testCase("123412423123");
    // testCase("a+b.c()+msd");
    // testCase(" \n", true);
    // testCase("{var a :=12;a}");
    // testCase("a-12=a+b+c*123-12=c+d", true);
    // testCase(R"(
    //     var a []f64 = []f64{1,2,3,4,5,6,7,8,9,10}
    //     func isPrime(n f64): f64 -> {
    //         var i f64 = 2
    //         var isP f64 = 1
    //         while (isP && n % i != 0 && i*i <= n) {
    //             i = i + 1
    //         }
    //         isP
    //     }
    //     func len(a []f64)(): f64->{
    //         len(a)
    //     }
    //     {
    //         var i f64 = 0
    //         while (i < a.len()) {
    //             if (isPrime(a[i])) {
    //                 print(a[i])
    //             }
    //             i = i + 1
    //         }
    //     }
    // )");
    // testCase(R"(
    //     func a():f64{
    //         b
    //     }
    //     var b:=1
    // )");
    testCase(R"(
        extern func print(a f64)
        func isPrime(n f64): f64 -> {
            var i f64 = 2
            var isP f64 = 1
            while (isP && n % i != 0 && i*i <= n)@main {
                i = i + 1
            }
            isP
        }
        func iter():f64->{
            0
        }
        func getIter(a []f64):func():f64{
            iter
        }
        var a []f64 = []f64{1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0,10.0}
        var iter := getIter(a)
        while (iter()) {
            print(iter())
        }
    )");
    testCase("var a:=1; a = if(a > 0) 1 else 0");

    return 0;
}