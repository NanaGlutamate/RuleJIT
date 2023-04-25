/**
 * @file cqmain.cpp
 * @author djw
 * @brief Test/CQ
 * @date 2023-03-28
 *
 * @details
 *
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-28</td><td>Initial version.</td></tr>
 * </table>
 */
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

    using CSValueMap = std::unordered_map<std::string, std::any>;

    RuleSetEngine engine;

    try {
        engine.buildFromFile(__PROJECT_ROOT_PATH "/doc/test_xml/car_rule.xml");
        engine.init();
    } catch (std::logic_error(e)) {
        std::cout << e.what() << std::endl;
        return 0;
    }
    for (int i = 0; i < 1000; i++) {
        engine.setInput(
            CSValueMap{{"Input1", std::vector<std::any>{double(1), double(3), double(5)}}, {"Input2", double(2)}});
        engine.tick();
        printCSValueMap(*(engine.getOutput()));
        // printCSValueMap(engine.data.cache);
        std::cout << std::endl;
    }

    return 0;
}