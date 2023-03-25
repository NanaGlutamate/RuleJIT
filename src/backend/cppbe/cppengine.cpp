#include "cppengine.h"

#include <iostream>

#include "rapidxml-1.13/rapidxml.hpp"

namespace {

using namespace rulejit;
using namespace rulejit::cppgen;

std::string CppStyleType(const TypeInfo &type) {
    // only support vector and base type
    if (type == NoInstanceType) {
        return "void";
    }
    if (type.isBaseType()) {
        return type.idents[0];
    }
    if (type.idents.size() == 2 && type.idents[0] == "[]") {
        return "std::vector<" + type.idents[1] + ">";
    }
    throw std::logic_error(std::format("unsupported type: {}", type.toString()));
}

std::string innerType(std::string type) {
    std::string tmp;
    while (type.back() == ']') {
        type.pop_back();
        if (type.back() != '[') {
            throw std::logic_error("Invalid type name: " + type + "]");
        }
        type.pop_back();
        tmp += "[]";
    }
    if (baseData.contains(type)) {
        tmp += "f64";
    } else {
        tmp += type;
    }
    return tmp;
}

std::vector<std::tuple<std::string, std::string>> assembleType(const std::vector<std::string> &src, MetaInfo &m) {
    std::vector<std::tuple<std::string, std::string>> ret;
    for (auto &&name : src) {
        if (auto it = m.varType.find(name); it != m.varType.end()) {
            ret.emplace_back(name, it->second);
        }
    }
    return ret;
}

struct file {
    std::string name;
    std::string content;
};

class CStyleString {
  public:
    char *s;
    CStyleString(size_t n) : s(new char[n]){};
    CStyleString(const CStyleString &s) = delete;
    void operator=(const CStyleString &s) = delete;
    CStyleString(const std::string &str) : s(new char[str.size() + 5]) { memcpy(s, str.c_str(), str.size() + 1); };
    ~CStyleString() { delete[] s; };
};

constexpr auto preDefines = R"()";

} // namespace

namespace rulejit::cppgen {

void CppEngine::buildFromSource(const std::string &srcXML) {
    using namespace rapidxml;
    using namespace templates;

    preDefines | lexer | parser | semantic;

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
        if (type == "Input" || type == "Output" || type == "Cache") {
            throw std::logic_error("Donot support user defined type name Input/Output/Cache");
        }
        auto &tar = data.typeDefines[type];
        for (auto member = typeDef->first_node("Variable"); member; member = member->next_sibling("Variable")) {
            tar.emplace_back(member->first_attribute("name")->value(), member->first_attribute("type")->value());
        }
    }
    auto meta = root->first_node("MetaInfo");
    for (auto input = meta->first_node("Inputs")->first_node("Param"); input; input = input->next_sibling("Param")) {
        std::string name = input->first_attribute("name")->value(), type = input->first_attribute("type")->value();
        data.inputVar.push_back(name);
        data.varType[name] = type;
        stack.scope.back().varDef.emplace(name, innerType(type) | lexer | TypeParser());
    }

    for (auto cache = meta->first_node("Caches")->first_node("Param"); cache; cache = cache->next_sibling("Param")) {
        std::string name = cache->first_attribute("name")->value(), type = cache->first_attribute("type")->value();
        data.cacheVar.push_back(name);
        data.varType[name] = type;
        stack.scope.back().varDef.emplace(name, innerType(type) | lexer | TypeParser());
    }

    for (auto output = meta->first_node("Outputs")->first_node("Param"); output;
         output = output->next_sibling("Param")) {
        std::string name = output->first_attribute("name")->value(), type = output->first_attribute("type")->value();
        data.outputVar.push_back(name);
        data.varType[name] = type;
        stack.scope.back().varDef.emplace(name, innerType(type) | lexer | TypeParser());
    }

    // gen subruleset defs
    std::string subs;
    size_t id = 0;
    for (auto subruleset = root->first_node("SubRuleSets")->first_node("SubRuleSet"); subruleset;
         subruleset = subruleset->next_sibling("SubRuleSet"), id++) {

        std::string expr = "{";
        auto rules = subruleset->first_node("Rules");
        size_t act = 0;
        for (auto rule = rules->first_node("Rule"); rule; rule = rule->next_sibling("Rule"), act++) {
            expr += std::string("if({") + rule->first_node("Condition")->first_node("Expression")->value() + "}){";
            for (auto assign = rule->first_node("Consequence")->first_node("Assignment"); assign;
                 assign = assign->next_sibling("Assignment")) {
                expr += std::string(assign->first_node("Target")->value()) + "={" +
                        assign->first_node("Value")->first_node("Expression")->value() + "};";
            }
            expr += std::to_string(act) + "}else ";
        }
        expr += "{-1}}";
        // std::cout<<expr;
        auto ast = std::unique_ptr<rulejit::ExprAST>(expr | lexer | parser) | semantic;
        subs += std::format(subRulesetDef, id, ast | codegen);
    }
    std::ofstream rulesetFile(outputPath + prefix + "ruleset.hpp");
    std::string subcall, subwrite;
    for (size_t i = 0; i < id; i++) {
        subcall += std::format(subRulesetCall, i);
        subwrite += std::format(subRulesetWrite, i);
    }
    rulesetFile << std::format(rulesetHpp, namespaceName, prefix, subcall, subwrite, subs, "");

    // gen typedefs
    std::string typedefs;
    std::ofstream typeDefFile(outputPath + prefix + "typedef.hpp");
    data.typeDefines.emplace("Input", assembleType(data.inputVar, data));
    data.typeDefines.emplace("Output", assembleType(data.outputVar, data));
    data.typeDefines.emplace("Cache", assembleType(data.cacheVar, data));
    for (auto &&[name, members] : data.typeDefines) {
        std::string member, serialize, deserialize;
        for (auto [token, type] : members) {
            std::string pre, suf;
            while (type.back() == ']') {
                type.pop_back();
                type.pop_back();
                pre += "std::vector<";
                suf += ">";
            }
            if (baseData.contains(type) && type != "float64") {
                pre += "typedReal<";
                suf += ">";
            }
            type = pre + type + suf;
            member += std::format(typeMember, type, token);
            serialize += std::format(typeSerialize, type, token);
            deserialize += std::format(typeDeserialize, type, token);
        }
        typedefs += std::format(typeDef, name, member, deserialize, serialize);
    }
    for (auto &&[name, type] : stack.global.typeDef) {
        // TODO: buffer << std::format(typeDef, );
        std::string member, serialize, deserialize;
        if (type.subTypes.size() + 1 != type.idents.size() || type.idents[0] != "struct") {
            throw std::logic_error(std::format("unsupported type: {}", type.toString()));
        }
        for (size_t i = 0; i < type.subTypes.size(); ++i) {
            member += std::format(typeMember, CppStyleType(type.subTypes[i]), type.idents[i + 1]);
            serialize += std::format(typeSerialize, CppStyleType(type.subTypes[i]), type.idents[i + 1]);
            deserialize += std::format(typeDeserialize, CppStyleType(type.subTypes[i]), type.idents[i + 1]);
        }
        typedefs += std::format(typeDef, name, member, deserialize, serialize);
    }
    typeDefFile << std::format(typeDefHpp, namespaceName, prefix, typedefs);

    std::ofstream funcDefFile(outputPath + prefix + "funcdef.hpp");
    std::string funcDefs, externDefs;
    for (auto &&[name, func] : stack.global.realFuncDefinition) {
        if (!stack.global.checkedFunc.contains(name)) {
            semantic.checkFunction(name);
        }
        std::string params;
        for (auto &&arg : func->params) {
            params += CppStyleType(*(arg->type)) + " " + arg->name + ", ";
        }
        if (!params.empty()) {
            params.erase(params.size() - 2, 2);
        }
        funcDefs += std::format(funcDef, CppStyleType(func->type->getReturnedType()), name, params, func->returnValue | codegen);
    }
    for(auto &&[name, type] : stack.global.externFuncDef) {
        std::string params;
        size_t param_cnt = type.subTypes.size() - type.isReturnedFunctionType() ? 1 : 0;
        for (size_t i = 0; i < param_cnt; ++i) {
            params += CppStyleType(type.subTypes[i]) + ", ";
        }
        if (!params.empty()) {
            params.erase(params.size() - 2, 2);
        }
        externDefs += std::format(externFuncDef, CppStyleType(type.getReturnedType()), name, params);
    }
    funcDefFile << std::format(funcDefHpp, namespaceName, prefix, funcDefs, externDefs);

    std::ofstream rulesetCppFile(outputPath + prefix + "ruleset.cpp");
    rulesetCppFile << std::format(rulesetCpp, namespaceName, prefix);

    std::ofstream cmakeTxtFile(outputPath + "CMakeLists.txt");
    cmakeTxtFile << std::format(CMakeListsTxt, namespaceName, prefix);

    std::ofstream testmainCppFile(outputPath + "testmain.cpp");
    testmainCppFile << testmainCpp;

    std::ofstream cqinterfaceHppFile(outputPath + "cqinterface.hpp");
    cqinterfaceHppFile << cqinterfaceHpp;
}

} // namespace rulejit::cppgen