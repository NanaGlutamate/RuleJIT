#include <iostream>

#include "ast/astprinter.hpp"
#include "frontend/parser.h"

#include "backend/cq/cqinterpreter.hpp"
#include "backend/cq/cqresourcehandler.h"
#include "backend/cq/cqrulesetengine.h"
#include "tools/printcsvaluemap.hpp"

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
    engine.buildFromFile(__PROJECT_ROOT_PATH"/doc/xml_design/example1.0.xml");

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