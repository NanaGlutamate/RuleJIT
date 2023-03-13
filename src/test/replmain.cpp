#include <fstream>
#include <iostream>

#include "ast/astprinter.hpp"
#include "frontend/parser.h"

#include "backend/cq/cqinterpreter.hpp"
#include "backend/cq/cqresourcehandler.h"
#include "backend/cq/cqrulesetengine.h"

void printCSValueMap(const std::unordered_map<std::string, std::any> &v) {
    using namespace std;
    cout << "{";
    bool start = false;
    for (auto &[k, v] : v) {
        if (!start) {
            start = true;
        } else {
            cout << ", ";
        }
        cout << k << ": ";
        if (v.type() == typeid(int8_t)) {
            cout << std::any_cast<int8_t>(v);
        } else if (v.type() == typeid(uint8_t)) {
            cout << std::any_cast<uint8_t>(v);
        } else if (v.type() == typeid(int16_t)) {
            cout << std::any_cast<int16_t>(v);
        } else if (v.type() == typeid(uint16_t)) {
            cout << std::any_cast<uint16_t>(v);
        } else if (v.type() == typeid(int32_t)) {
            cout << std::any_cast<int32_t>(v);
        } else if (v.type() == typeid(uint32_t)) {
            cout << std::any_cast<uint32_t>(v);
        } else if (v.type() == typeid(int64_t)) {
            cout << std::any_cast<int64_t>(v);
        } else if (v.type() == typeid(uint64_t)) {
            cout << std::any_cast<uint64_t>(v);
        } else if (v.type() == typeid(float)) {
            cout << std::any_cast<float>(v);
        } else if (v.type() == typeid(double)) {
            cout << std::any_cast<double>(v);
        } else if (v.type() == typeid(std::unordered_map<std::string, std::any>)) {
            printCSValueMap(std::any_cast<std::unordered_map<std::string, std::any>>(v));
        } else if (v.type() == typeid(std::vector<std::any>)) {
            auto tmp = std::any_cast<std::vector<std::any>>(v);
            cout << "{";
            bool start_ = false;
            for (auto &&item : tmp) {
                if (!start_) {
                    start_ = true;
                } else {
                    cout << ", ";
                }
                printCSValueMap(std::any_cast<std::unordered_map<std::string, std::any>>(item));
            }
            cout << "}";
        } else {
            cout << "unknown";
        }
    }
    cout << "}";
}

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