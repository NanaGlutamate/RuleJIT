#include "backend/pybe/pybe.h"
#include "backend/pybe/pycodegen.hpp"

int main(){
    using namespace rulejit;
    using namespace rulejit::pybe;
    PYEngine engine;
    engine.outputPath = __PROJECT_ROOT_PATH"/bin/generated_py/";
    engine.buildFromFile(__PROJECT_ROOT_PATH"/doc/test_xml/simple.xml");
    return 0;
}