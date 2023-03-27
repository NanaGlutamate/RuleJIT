/**
 * @file cppengine.cpp
 * @author djw
 * @brief CQ/CPPBE/Cpp Engine
 * @date 2023-03-27
 * 
 * @details Includes main logic of cpp-backend, such as
 * XML-parsering and code generate.
 * 
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-27</td><td>Initial version.</td></tr>
 * </table>
 */
#include "cppengine.h"

#include <iostream>

#include "rapidxml-1.13/rapidxml.hpp"

namespace {

using namespace rulejit;
using namespace rulejit::cppgen;

/**
 * @brief transform inner type string to cpp style string
 * 
 * @param type inner type string
 * @return std::string cpp style string
 */
std::string CppStyleType(const TypeInfo &type) {
    // only support vector and base type
    if (type == RealType) {
        return "double";
    }
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

/**
 * @brief transform inner param type string to cpp style string
 * @attention will use const reference type for complex type and array type
 * 
 * @param type inner type string
 * @return std::string cpp style string
 */
std::string CppStyleParamType(const TypeInfo &type) {
    // only support vector and base type
    if (type == NoInstanceType) {
        return "void";
    }
    if (type.isBaseType() && baseData.contains(type.idents[0])) {
        return type.idents[0];
    }
    if (type.isBaseType()) {
        return "const " + type.idents[0] + "&";
    }
    if (type.idents.size() == 2 && type.idents[0] == "[]") {
        return "const std::vector<" + type.idents[1] + ">&";
    }
    throw std::logic_error(std::format("unsupported type: {}", type.toString()));
}

/**
 * @brief transform cpp style type string to inner type string
 * 
 * @param type cpp style type string
 * @return std::string inner type string
 */
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

/**
 * @brief for each name in src, look up type defines in meta info,
 * select type string of given name, assemble them in std::pair and emplace into std::vector,
 * then return the vector. only unsed for _Input/_Output/_Cache type defines
 * 
 * @param src vector containes name of variable
 * @param m meta info which containes type defines
 * @return std::vector<std::tuple<std::string, std::string>> assembled type which can used in type defines
 */
std::vector<std::tuple<std::string, std::string>> assembleType(const std::vector<std::string> &src, MetaInfo &m) {
    std::vector<std::tuple<std::string, std::string>> ret;
    for (auto &&name : src) {
        if (auto it = m.varType.find(name); it != m.varType.end()) {
            ret.emplace_back(name, it->second);
        }
    }
    return ret;
}

/**
 * @brief writable c style string used for rapidxml to process
 * 
 */
class CStyleString {
  public:
    char *s;
    CStyleString(size_t n) : s(new char[n]){};
    CStyleString(const CStyleString &s) = delete;
    void operator=(const CStyleString &s) = delete;
    CStyleString(const std::string &str) : s(new char[str.size() + 1]) { memcpy(s, str.c_str(), str.size() + 1); };
    ~CStyleString() { delete[] s; };
};

/**
 * @brief pre-defines which will be process before ruleset xml,
 * can add some tool functions and types
 * 
 */
constexpr auto preDefines = R"(
extern func sin (i f64):f64
type Vector3 struct {
    x f64;
    y f64;
    z f64;
}
func addv(a Vector3, b Vector3):Vector3 {
    Vector3{a.x + b.x, a.y + b.y, a.z + b.z};
}
)";

} // namespace

namespace rulejit::cppgen {

void CppEngine::buildFromSource(const std::string &srcXML) {
    using namespace rapidxml;
    using namespace templates;

    std::set<std::string> notGenerate;

    notGenerate.insert(preDefines | lexer | parser | semantic);

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
            throw std::logic_error("Donot support user defined type name _Input/_Output/_Cache");
        }
        auto &tar = data.typeDefines[type];
        for (auto member = typeDef->first_node("Variable"); member; member = member->next_sibling("Variable")) {
            tar.emplace_back(member->first_attribute("name")->value(), member->first_attribute("type")->value());
        }
    }
    auto meta = root->first_node("MetaInfo");

    /// collect input/cache/output vars, and if element <Param> has sub element
    /// named "Value", add assignment to preprocessOriginal
    std::string preprocessOriginal = "{";

    for (auto input = meta->first_node("Inputs")->first_node("Param"); input; input = input->next_sibling("Param")) {
        std::string name = input->first_attribute("name")->value(), type = input->first_attribute("type")->value();
        data.inputVar.push_back(name);
        data.varType[name] = type;
        context.scope.back().varDef.emplace(name, innerType(type) | lexer | TypeParser());
        if (auto p = input->first_node("Value"); p) {
            preprocessOriginal += std::string(input->first_attribute("name")->value()) + "={" +
                          p->first_node("Expression")->value() + "};";
        }
    }

    for (auto cache = meta->first_node("Caches")->first_node("Param"); cache; cache = cache->next_sibling("Param")) {
        std::string name = cache->first_attribute("name")->value(), type = cache->first_attribute("type")->value();
        data.cacheVar.push_back(name);
        data.varType[name] = type;
        context.scope.back().varDef.emplace(name, innerType(type) | lexer | TypeParser());
        if (auto p = cache->first_node("Value"); p) {
            preprocessOriginal += std::string(cache->first_attribute("name")->value()) + "={" +
                          p->first_node("Expression")->value() + "};";
        }
    }

    for (auto output = meta->first_node("Outputs")->first_node("Param"); output;
         output = output->next_sibling("Param")) {
        std::string name = output->first_attribute("name")->value(), type = output->first_attribute("type")->value();
        data.outputVar.push_back(name);
        data.varType[name] = type;
        context.scope.back().varDef.emplace(name, innerType(type) | lexer | TypeParser());
        if (auto p = output->first_node("Value"); p) {
            preprocessOriginal += std::string(output->first_attribute("name")->value()) + "={" +
                          p->first_node("Expression")->value() + "};";
        }
    }

    preprocessOriginal += "}";
    // parse preprocessOriginal, add generated code to RuleSet::Tick() in front of subruleset calls
    auto pre = std::unique_ptr<rulejit::ExprAST>(preprocessOriginal | lexer | parser) | semantic;
    notGenerate.insert(pre);
    std::string preprocess = context.global.realFuncDefinition[pre]->returnValue | codegen;

    // generate subruleset defs
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
        auto astName = std::unique_ptr<rulejit::ExprAST>(expr | lexer | parser) | semantic;
        notGenerate.insert(astName);
        auto &ast = context.global.realFuncDefinition[astName]->returnValue;
        subs += std::format(subRulesetDef, id, ast | codegen);
    }
    std::ofstream rulesetFile(outputPath + prefix + "ruleset.hpp");
    std::string subcall, subwrite;
    for (size_t i = 0; i < id; i++) {
        subcall += std::format(subRulesetCall, i);
        subwrite += std::format(subRulesetWrite, i);
    }
    rulesetFile << std::format(rulesetHpp, namespaceName, prefix, subcall, subwrite, subs, "", preprocess);

    // generate typedefs
    std::string typedefs;
    std::ofstream typeDefFile(outputPath + prefix + "typedef.hpp");
    data.typeDefines.emplace("_Input", assembleType(data.inputVar, data));
    data.typeDefines.emplace("_Output", assembleType(data.outputVar, data));
    data.typeDefines.emplace("_Cache", assembleType(data.cacheVar, data));
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
    for (auto &&[name, type] : context.global.typeDef) {
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

    // generate func def
    std::ofstream funcDefFile(outputPath + prefix + "funcdef.hpp");
    std::string funcDefs, externDefs;
    for (auto &&[name, func] : context.global.realFuncDefinition) {
        if (notGenerate.contains(name)) {
            continue;
        }
        if (!context.global.checkedFunc.contains(name)) {
            semantic.checkFunction(name);
        }
        std::string params;
        for (auto &&arg : func->params) {
            params += CppStyleParamType(*(arg->type)) + " " + arg->name + ", ";
        }
        if (!params.empty()) {
            params.erase(params.size() - 2, 2);
        }
        funcDefs +=
            std::format(funcDef, CppStyleType(func->funcType->getReturnedType()), name, params,
                        (func->funcType->isReturnedFunctionType() ? "return" : "") + (func->returnValue | codegen));
    }
    // generate extern func type
    for (auto &&[name, type] : context.global.externFuncDef) {
        std::string params;
        size_t param_cnt = type.subTypes.size() - type.isReturnedFunctionType() ? 1 : 0;
        for (size_t i = 0; i < param_cnt; ++i) {
            // TODO: array param?
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