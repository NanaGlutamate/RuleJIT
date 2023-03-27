#pragma once

#include <fstream>
#include <list>

#include "backend/cq/cqinterpreter.hpp"
#include "backend/cq/cqresourcehandler.h"
#include "frontend/lexer.h"
#include "frontend/parser.h"

namespace rulejit::cq {

struct SubRuleSet {
    SubRuleSet(DataStore &data) : handler(data), interpreter(handler), subruleset(nullptr) {}
    SubRuleSet() = delete;
    ResourceHandler handler;
    CQInterpreter interpreter;
    std::unique_ptr<ExprAST> subruleset;
};

struct RuleSet {
    std::list<SubRuleSet> subRuleSets;
};

struct RuleSetEngine {
    RuleSetEngine() : data(), ruleset(), lexer(), parser(), preProcess(data) {}
    //! build from XML file
    //! @param srcXML string of content of XML file
    //! @return none.
    void buildFromSource(const std::string &srcXML);

    //! build from XML file
    //! @param XMLFilePath string of path of XML file
    //! @return none.
    void buildFromFile(const std::string &XMLFilePath) {
        using namespace std;
        std::ifstream file(XMLFilePath);
        std::string buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        buildFromSource(buffer);
    }
    void init() { data.Init(); }
    void tick() {
        preProcess.subruleset | preProcess.interpreter;
        preProcess.handler.writeBack();
        preProcess.interpreter.symbolStack = {{{}}};

        for (auto &&s : ruleset.subRuleSets) {
            s.subruleset | s.interpreter;
        }
        for (auto &&s : ruleset.subRuleSets) {
            s.handler.writeBack();
            s.interpreter.symbolStack = {{{}}};
        }
    }
    void setInput(const std::unordered_map<std::string, std::any> &input) { data.SetInput(input); }
    std::unordered_map<std::string, std::any> *getOutput() { return data.GetOutput(); }

    // private:
    DataStore data;
    RuleSet ruleset;
    ExpressionLexer lexer;
    ExpressionParser parser;
    SubRuleSet preProcess;
};

} // namespace rulejit::cq