#include "cqrulesetengine.h"

#include <iostream>

#include "rapidxml-1.13/rapidxml.hpp"

namespace {

class CStyleString {
  public:
    char *s;
    CStyleString(size_t n) : s(new char[n]){};
    CStyleString(const CStyleString &s) = delete;
    void operator=(const CStyleString &s) = delete;
    CStyleString(const std::string &str) : s(new char[str.size() + 5]) { memcpy(s, str.c_str(), str.size() + 1); };
    ~CStyleString() { delete[] s; };
};

} // namespace

namespace rulejit::cq {

void RuleSetEngine::buildFromSource(const std::string &srcXML) {
    using namespace rapidxml;
    xml_document<> doc;
    CStyleString s(srcXML);
    doc.parse<parse_default>(s.s);
    auto root = doc.first_node("RuleSet");
    if (auto it = root->first_attribute("version"); !it || it->value() != std::string("1.0")) {
        throw std::runtime_error("Unsupported version of RuleSet");
    }
    for (auto typeDef = root->first_node("TypeDefines")->first_node("TypeDefine"); typeDef;
         typeDef = typeDef->next_sibling("TypeDefine")) {
        std::string type = typeDef->first_attribute("type")->value();
        auto &tar = data.typeDefines[type];
        for (auto member = typeDef->first_node("Variable"); member; member = member->next_sibling("Variable")) {
            tar[member->first_attribute("name")->value()] = member->first_attribute("type")->value();
        }
    }
    auto meta = root->first_node("MetaInfo");

    std::string preprocess = "{";

    // collect inputs, caches and outputs variables and their types, store them in data.varType
    for (auto input = meta->first_node("Inputs")->first_node("Param"); input; input = input->next_sibling("Param")) {
        data.inputVar.push_back(input->first_attribute("name")->value());
        data.varType[input->first_attribute("name")->value()] = input->first_attribute("type")->value();
        if(auto p = input->first_node("Value"); p){
            preprocess += std::string(input->first_attribute("name")->value()) + "={" + p->first_node("Expression")->value() + "};";
        }
    }

    for (auto cache = meta->first_node("Caches")->first_node("Param"); cache; cache = cache->next_sibling("Param")) {
        data.cacheVar.push_back(cache->first_attribute("name")->value());
        data.varType[cache->first_attribute("name")->value()] = cache->first_attribute("type")->value();
        if(auto p = cache->first_node("Value"); p){
            preprocess += std::string(cache->first_attribute("name")->value()) + "={" + p->first_node("Expression")->value() + "};";
        }
    }

    for (auto output = meta->first_node("Outputs")->first_node("Param"); output;
         output = output->next_sibling("Param")) {
        data.outputVar.push_back(output->first_attribute("name")->value());
        data.varType[output->first_attribute("name")->value()] = output->first_attribute("type")->value();
        if(auto p = output->first_node("Value"); p){
            preprocess += std::string(output->first_attribute("name")->value()) + "={" + p->first_node("Expression")->value() + "};";
        }
    }

    preprocess += "}";
    // generate preprocess subruleset, which will be called tick() and writeBack() before all subruleset
    preProcess.subruleset = preprocess | lexer | parser;

    // for each subruleset node, generate ast and store it in ruleset
    for (auto subruleset = root->first_node("SubRuleSets")->first_node("SubRuleSet"); subruleset;
         subruleset = subruleset->next_sibling("SubRuleSet")) {
        ruleset.subRuleSets.emplace_back(data);
        SubRuleSet &tmp = ruleset.subRuleSets.back();
        std::string expr = "{";
        auto rules = subruleset->first_node("Rules");
        for (auto rule = rules->first_node("Rule"); rule; rule = rule->next_sibling("Rule")) {
            expr += std::string("if({") + rule->first_node("Condition")->first_node("Expression")->value() + "}){";
            for (auto assign = rule->first_node("Consequence")->first_node("Assignment"); assign;
                 assign = assign->next_sibling("Assignment")) {
                expr += std::string(assign->first_node("Target")->value()) + "={" +
                        assign->first_node("Value")->first_node("Expression")->value() + "};";
            }
            expr += "}else ";
        }
        expr += "{}}";
        tmp.subruleset = expr | lexer | parser;
    }
}

} // namespace rulejit::cq