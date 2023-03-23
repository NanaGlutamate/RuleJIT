#include <fstream>
#include <iostream>

#include "ast/astprinter.hpp"
#include "frontend/parser.h"

#include "backend/cq/cqinterpreter.hpp"
#include "backend/cq/cqresourcehandler.h"
#include "backend/cq/cqrulesetengine.h"
#include "tools/printcsvaluemap.hpp"

int main() {
    using namespace std;
    using namespace rulejit;
    using namespace rulejit::cq;

    using CSValueMap = std::unordered_map<std::string, std::any>;

    static DataStore data;

    static ExpressionLexer lexer;
    static ExpressionParser parser;
    static ASTPrinter printer;
    static CQInterpreter interpreter;
    static ResourceHandler handler(data);

    data.Init();
    interpreter.setResourceHandler(&handler);
    data.typeDefines.emplace(
        "Vector3", std::unordered_map<std::string, std::string>{{"x", "float64"}, {"y", "float64"}, {"z", "float64"}});

    while (true) {
        std::string in;
        cout << ">>> ";
        int cnt = 0;
        while (true) {
            std::string tmp;
            getline(cin, tmp);
            in += tmp + "\n";
            for (auto &&c : tmp) {
                if (c == '{' || c == '(') {
                    cnt++;
                } else if (c == '}' || c == ')') {
                    cnt--;
                }
            }
            if (cnt == 0) {
                break;
            }
            cout << "... ";
        }
        if (in == "\n")
            continue;
        if (in.size() > 5 && in[0] == 'f' && in[1] == 'u' && in[2] == 'n' && in[3] == 'c' && in[4] == ' ') {
            in = "{" + in + "}";
        }
        try {
            auto expr = in | lexer | parser;
            expr | interpreter;
            if (interpreter.returned.type == CQInterpreter::Value::VALUE) {
                cout << interpreter.returned.value;
                cout << endl;
            } else if (interpreter.returned.type == CQInterpreter::Value::TOKEN) {
                if (handler.isBaseType(interpreter.returned.token)) {
                    interpreter.getReturnedValue();
                    cout << interpreter.returned.value;
                } else {
                    auto ret = handler.assemble(interpreter.returned.token);
                    auto type = std::get<1>(handler.buffer[interpreter.returned.token]);
                    if (!data.isArray(type)) {
                        printCSValueMap(any_cast<CSValueMap>(ret));
                    } else {
                        printCSValueMap(CSValueMap{{"list", ret}});
                    }
                }
                cout << endl;
            }
            interpreter.returned.type = CQInterpreter::Value::EMPTY;
        } catch (std::exception &e) {
            cout << e.what() << endl;
            while (interpreter.symbolStack.size() > 1)
                interpreter.symbolStack.pop_back();
        }
    }

    printCSValueMap(*(data.GetOutput()));

    return 0;
}