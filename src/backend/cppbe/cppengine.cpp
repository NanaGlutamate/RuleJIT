/**
 * @file cppengine.cpp
 * @author djw
 * @brief CQ/CPPBE/Cpp Engine
 * @date 2023-03-27
 *
 * @details Includes main logic of cpp-backend, such as
 * XML-parsering and code generation.
 *
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-27</td><td>Initial version.</td></tr>
 * <tr><td>djw</td><td>2023-03-29</td><td>Move XML-Parsering to frontend/ruleset</td></tr>
 * <tr><td>djw</td><td>2023-04-18</td><td>make every <Value> a single subruleset</td></tr>
 * <tr><td>djw</td><td>2023-04-18</td><td>make <Value> a single subruleset</td></tr>
 * </table>
 */
#include <iostream>

#include "backend/cppbe/template.hpp"
#include "cppengine.h"
#include "rapidxml-1.13/rapidxml.hpp"
#include "tools/seterror.hpp"
#include "defines/marco.hpp"

namespace {

using namespace rulejit;
using namespace rulejit::cppgen;

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
    
    if (type.isBaseType()) {
        auto tmp = type.getBaseTypeString();
        if (ruleset::baseNumericalData.contains(tmp)) {
            return "typedReal<" + tmp + ">";
        } else {
            return "const " + tmp + "&";
        }
    }
    if (type.isArrayType()) {
        auto tmp = type.getElementType();
        return "const " + SubRuleSetCodeGen::CppStyleType(type) + "&";
    }
    error(std::format("unsupported type: {}", type.toString()));
}

/**
 * @brief for each name in src, look up type defines in meta info,
 * select type string of given name, assemble them in std::pair and emplace into std::vector,
 * then return the vector. only unsed for _Input/_Output/_Cache type defines
 *
 * @param src vector contains name of variable
 * @param m meta info which contains type defines
 * @return std::vector<std::tuple<std::string, std::string>> assembled type which can used in type defines
 */
std::vector<std::tuple<std::string, std::string>> assembleType(const std::vector<std::string> &src,
                                                               ruleset::RuleSetMetaInfo &m) {
    std::vector<std::tuple<std::string, std::string>> ret;
    for (auto &&name : src) {
        if (auto it = m.varType.find(name); it != m.varType.end()) {
            ret.emplace_back(name, it->second);
        }
    }
    return ret;
}

} // namespace

namespace rulejit::cppgen {

void CppEngine::buildFromSource(const std::string &srcXML) {

    using namespace rulejit::cppgen::templates;
    using namespace rulejit::ruleset;

    // discard statements in preDefines
    // TODO: execute preDefines once(in RuleSet::Init()) to handle init value?
    auto [preDefines, preProcess, subRuleSets] = RuleSetParser::readSource(srcXML, context, data);

    std::set<std::string> notGenerate{preProcess.begin(), preProcess.end()};
    notGenerate.emplace(preDefines);

    // collect subruleset defs
    std::string subs;
    size_t id = 0;
    for (auto& astName : preProcess) {
        notGenerate.insert(astName);
        auto &ast = context.global.realFuncDefinition[astName]->returnValue;
        std::string writeBacks;
        for (size_t atom = 0; atom < data.modifiedValue[id].size(); ++atom){
            std::string members;
            for (auto& name : data.modifiedValue[id][atom]) {
                if(std::ranges::find(data.cacheVar, name) == data.cacheVar.end()){
                    continue;
                }
                members += std::format(subRulesetWriteCaseMember, name);
            }
            writeBacks += std::format(subRulesetWriteCase, atom, members);
        }
        subs += std::format(subRulesetDef, id++, ast | codegen, writeBacks);
    }
    size_t preID = id;
    for (auto& astName : subRuleSets) {
        notGenerate.insert(astName);
        auto &ast = context.global.realFuncDefinition[astName]->returnValue;
        std::string writeBacks;
        for (size_t atom = 0; atom < data.modifiedValue[id].size(); ++atom){
            std::string members;
            for (auto& name : data.modifiedValue[id][atom]) {
                if(std::ranges::find(data.cacheVar, name) == data.cacheVar.end()){
                    continue;
                }
                members += std::format(subRulesetWriteCaseMember, name);
            }
            writeBacks += std::format(subRulesetWriteCase, atom, members);
        }
        subs += std::format(subRulesetDef, id++, ast | codegen, writeBacks);
    }
    std::string precall, prewrite, subcall, subwrite;
    for (size_t i = 0; i < preID; i++) {
        precall += std::format(subRulesetCall, i);
        prewrite += std::format(subRulesetWrite, i);
    }
    for (size_t i = preID; i < id; i++) {
        subcall += std::format(subRulesetCall, i);
        subwrite += std::format(subRulesetWrite, i);
    }

    // collect typedefs
    std::string typedefs;
    if (data.typeDefines.contains("_Input") || data.typeDefines.contains("_Output") ||
        data.typeDefines.contains("_Cache")) {
        error("CPP-Backend donot support user defined type name _Input/_Output/_Cache");
    }
    data.typeDefines.emplace("_Input", assembleType(data.inputVar, data));
    data.typeDefines.emplace("_Output", assembleType(data.outputVar, data));
    data.typeDefines.emplace("_Cache", assembleType(data.cacheVar, data));
    // collect typedefs in XML
    for (auto &&[name, members] : data.typeDefines) {
        std::string member, serialize, deserialize;
        for (auto& [token, _type] : members) {
            auto type = _type;
            std::string pre, suf;
            while (type.back() == ']') {
                type.erase(type.end() - 2, type.end());
                pre += "std::vector<";
                suf += ">";
            }
            if (baseNumericalData.contains(type)) {
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
    // collect typedefs in Expression
    for (auto &&[name, type] : context.global.typeDef) {
        if (data.typeDefines.contains(name)) {
            // means type is defined in XML and add to context, so skip
            continue;
        }
        std::string member, serialize, deserialize;
        if (type.getIdent() != "struct") {
            error(std::format("unsupported type: {}", type.toString()));
        }
        auto& subTypes = type.getSubTypes();
        auto& tokens = type.getTokens();
        for (size_t i = 0; i < subTypes.size(); ++i) {
            member += std::format(typeMember, SubRuleSetCodeGen::CppStyleType(subTypes[i]), tokens[i + 1]);
            serialize += std::format(typeSerialize, SubRuleSetCodeGen::CppStyleType(subTypes[i]), tokens[i + 1]);
            deserialize += std::format(typeDeserialize, SubRuleSetCodeGen::CppStyleType(subTypes[i]), tokens[i + 1]);
        }
        typedefs += std::format(typeDef, name, member, deserialize, serialize);
    }

    // collect func def
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
            std::format(funcDef, SubRuleSetCodeGen::CppStyleType(func->funcType->getReturnedType()), codegen.toLegalName(name), params,
                        (func->funcType->isReturnedFunctionType() ? "return" : "") + (func->returnValue | codegen));
    }
    // collect extern func type
    for (auto &&[name, type] : context.global.externFuncDef) {
        std::string params;
        size_t param_cnt = type.getParamCount();
        auto& subTypes = type.getSubTypes();
        for (size_t i = 0; i < param_cnt; ++i) {
            params += SubRuleSetCodeGen::CppStyleType(subTypes[i]) + ", ";
        }
        if (!params.empty()) {
            params.erase(params.size() - 2, 2);
        }
        // TODO: extern func may have bugs, do not support for now
        // externDefs += std::format(externFuncDef, CppStyleType(type.getReturnedType()), name, params);
    }

    // generate ruleset.hpp
    std::ofstream rulesetFile(outputPath + prefix + "ruleset.hpp");
    rulesetFile << std::format(rulesetHpp, namespaceName, prefix, subcall, subwrite, subs, "", precall, prewrite);

    // generate typedef.hpp
    std::ofstream typeDefFile(outputPath + prefix + "typedef.hpp");
    typeDefFile << std::format(typeDefHpp, namespaceName, prefix, typedefs);

    // generate funcdef.hpp
    std::ofstream funcDefFile(outputPath + prefix + "funcdef.hpp");
    funcDefFile << std::format(funcDefHpp, namespaceName, prefix, funcDefs, externDefs);

    // generate ruleset.cpp
    std::ofstream rulesetCppFile(outputPath + prefix + "ruleset.cpp");
    rulesetCppFile << std::format(rulesetCpp, namespaceName, prefix);

    // generate CMakeLists.txt
    std::ofstream cmakeTxtFile(outputPath + "CMakeLists.txt");
    cmakeTxtFile << std::format(CMakeListsTxt, namespaceName, prefix);

    // generate testmain.cpp
    std::ofstream testmainCppFile(outputPath + "testmain.cpp");
    testmainCppFile << testmainCpp;

    // generate cqinterface.hpp
    std::ofstream cqinterfaceHppFile(outputPath + "cqinterface.hpp");
    cqinterfaceHppFile << cqinterfaceHpp;
}

} // namespace rulejit::cppgen