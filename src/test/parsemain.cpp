#include "ast/astprinter.hpp"
#include "frontend/parser.h"

#define CATCH_EXCEPTION

int printAST(const std::string &s, bool expectError = false) {
    using namespace std;
    using namespace rulejit;

    static ExpressionLexer lexer;
    static ExpressionParser parser;
    static ASTPrinter printer;

    #ifdef CATCH_EXCEPTION
    try {
    #endif
        std::unique_ptr<AST> ast = s | lexer | parser;
        std::string ast_str = ast | printer;
    #ifdef CATCH_EXCEPTION
        if(!expectError){cout << "PASSED ";}
    #endif
        cout << ast_str << endl;
    #ifdef CATCH_EXCEPTION
    } catch (std::logic_error &e) {if(expectError){cout << "PASSED ";} cout << "ERROR: " << e.what() << endl;
    #endif
        if (lexer.errorHandler.err)
            cout << std::format("     in parsing string: {}, {}", lexer.top(), lexer.errorHandler.info) << endl;
        if (parser.errorHandler.err)
            cout << std::format("     in parsing token: {}({}), {}", to_string(parser.errorHandler.type),
                                parser.errorHandler.token, parser.errorHandler.info)
                 << endl;
    #ifdef CATCH_EXCEPTION
    }
    #endif

    return 0;
}

int main() {
    using namespace std;
    using namespace rulejit;

    // std::string tmp;
    // ExpressionLexer lexer;
    // ExpressionParser parser;
    // ASTPrinter printer;
    // std::unique_ptr<AST> ast;
    // tmp = "{var a i32=12;a}";

    printAST("123412423123");
    printAST("a+b.c()+msd");
    printAST(" \n", true);
    printAST("{var a i32=12;a}");
    printAST("a+b\n+c");
    printAST("(a+b\n+c)");
    printAST("a+b;+c", true);
    printAST("(a+b;+c)", true);
    printAST("a-12=a+b+c*123-12=c+d", true);
    printAST(R"({
        var a []i32 = []i32{1,2,3,4,5,6,7,8,9,10}
        func isPrime(i32 n): i32 -> {
            var i i32 = 2
            var isP i32 = 1
            while (isP && n % i != 0 && i*i <= n) {
                i = i + 1
            }
            isP
        }
        func len(a []i32)(): i32->{
            len(a)
        }
        {
            var i i32 = 0
            while (i < a.len()) {
                if (isPrime(a[i])) {
                    print(a[i])
                }
                i = i + 1
            }
        }
    })");

    return 0;
}