#include <iostream>

#include "ast/astprinter.hpp"
#include "frontend/parser.h"

#include "backend/cq/cqinterpreter.hpp"
#include "backend/cq/cqresourcehandler.h"
#include "backend/cq/cqrulesetengine.h"

void printCSValueMap(const std::unordered_map<std::string, std::any> &v) {
    using namespace std;
    cout << "{";
    bool start=false;
    for (auto &[k, v] : v) {
        if(!start){
            start=true;
        }else{
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
            for(auto&& item : tmp){
                if(!start_){
                    start_ = true;
                }else{
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
    using namespace rulejit;
    using namespace rulejit::cq;

    // static DataStore data;

    // static ExpressionLexer lexer;
    // static ExpressionParser parser;
    // static ASTPrinter printer;
    // static CQInterpreter interpreter;
    // static ResourceHandler handler(data);

    using CSValueMap = std::unordered_map<std::string, std::any>;

    // data.varType = {{"vi", "Vector3"}, {"vo", "Vector3"}};
    // data.inputVar = {"vi"};
    // data.outputVar = {"vo"};
    // data.typeDefines = {{"Vector3", {{"x", "float64"}, {"y", "float64"}, {"z", "float64"}}}};

    // data.Init();
    // interpreter.setResourceHandler(&handler);
    // data.SetInput({{"vi", CSValueMap{{"x", 1.0}, {"y", 2.0}, {"z", 3.0}}}});

    // auto expr = "{func norm1(v Vector3):f64->v.x+v.y+v.z;vo.x = norm1(vi); vo.y = 2 * vi.y; vo.z = 2 * vi.z;}" | lexer | parser;

    // // std::cout << (expr | printer);

    // expr | interpreter;

    // handler.writeBack();

    // printCSValueMap(*(data.GetOutput()));

    RuleSetEngine engine;
    engine.buildFromFile("D:/Desktop/FinalProj/Code/RuleJIT/doc/xml_design/example1.0.xml");

    engine.init();
    for(int i=0; i<1000; i++){
        engine.setInput(CSValueMap{{"Input1", double(1)}});
        engine.tick();
        printCSValueMap(*(engine.getOutput()));
        printCSValueMap(engine.data.cache);
        std::cout << std::endl;
    }

    return 0;
}