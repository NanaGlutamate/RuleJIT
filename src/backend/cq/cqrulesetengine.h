#pragma once

#include <fstream>
#include <list>

#include "backend/cq/cqinterpreter.hpp"
#include "backend/cq/cqresourcehandler.h"
#include "frontend/lexer.h"
#include "frontend/parser.h"

namespace rulejit::cq {

struct SubRuleSet {
    SubRuleSet(DataStore& data):handler(data), interpreter(), subruleset(nullptr){
        interpreter.setResourceHandler(&handler);
    }
    ResourceHandler handler;
    CQInterpreter interpreter;
    std::unique_ptr<ExprAST> subruleset;
};

struct RuleSet {
    std::list<SubRuleSet> subRuleSets;
};

struct RuleSetEngine {    
    //! build from XML file
    //! @param srcXML string of content of XML file
    //! @return none.
    void buildFromSource(const std::string& srcXML);

    //! build from XML file
    //! @param XMLFilePath string of path of XML file
    //! @return none.
    void buildFromFile(const std::string& XMLFilePath){
        using namespace std;
        std::ifstream file(XMLFilePath);
        std::string buffer;    
        while(!(file.eof())){
            string tmp;
            getline(file, tmp);
            buffer.append(tmp);
        }
        buildFromSource(buffer);
    }
    void init(){
        data.Init();
    }
    void tick(){
        for(auto&& s : ruleset.subRuleSets){
            s.subruleset | s.interpreter;
        }
        for(auto&& s: ruleset.subRuleSets){
            s.handler.writeBack();
        }
    }
    void setInput(const std::unordered_map<std::string, std::any>& input){
        data.SetInput(input);
    }
    std::unordered_map<std::string, std::any>* getOutput(){
        return data.GetOutput();
    }

  //private:
    DataStore data;
    RuleSet ruleset;
    ExpressionLexer lexer;
    ExpressionParser parser;
};

} // namespace rulejit::cq