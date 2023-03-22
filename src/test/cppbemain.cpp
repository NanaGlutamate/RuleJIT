#include "backend/cppbe/cppengine.h"

int main() {
    using namespace rulejit;
    using namespace rulejit::cppgen;
    CppEngine engine;
    engine.setOutputPath(__PROJECT_ROOT_PATH"/doc/generated/");
    engine.buildFromFile(__PROJECT_ROOT_PATH"/doc/xml_design/example1.0.xml");

    return 0;
}