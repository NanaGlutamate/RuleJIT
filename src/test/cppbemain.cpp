/**
 * @file cppbemain.cpp
 * @author djw
 * @brief Test/CPP-Backend
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
#include "backend/cppbe/cppengine.h"

int main() {
    using namespace rulejit;
    using namespace rulejit::cppgen;
    CppEngine engine;
    engine.setOutputPath(__PROJECT_ROOT_PATH"/bin/generated/");
    engine.setNamespaceName("ruleset");
    engine.setPrefix("");
    
    engine.buildFromFile(__PROJECT_ROOT_PATH"/doc/xml_design/example1.0.xml");
    return 0;
}