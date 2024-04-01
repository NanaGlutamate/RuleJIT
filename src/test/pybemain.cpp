#include "backend/pybe/pybe.h"

int main(){
    using namespace rulejit;
    using namespace rulejit::pybe;
    PYEngine engine;
    engine.outputPath = "D:/Desktop/FinalProj/Code/train/";
    engine.buildFromFile("D:/Desktop/FinalProj/Code/simplecq/config/rule_attackside.xml");
    return 0;
}