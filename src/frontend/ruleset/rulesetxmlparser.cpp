/**
 * @file rulesetxmlparser.cpp
 * @author djw
 * @brief
 * @date 2023-05-09
 *
 * @details
 *
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-05-09</td><td>Initial version.</td></tr>
 * </table>
 */
#include "rulesetxmlparser.h"

#include <string_view>

#include "rapidxml-1.13/rapidxml.hpp"
#include "tools/stringprocess.hpp"

namespace {

/// @brief Supported data types in ruleset XML
inline static std::set<std::string> baseData{
    "bool",  "int8",   "uint8",   "int16",   "uint16",   "int32",  "uint32",
    "int64", "uint64", "float32", "float64", "float128", "string",
};

/// @brief Supported numerical data types in ruleset XML
inline static std::set<std::string> baseNumericalData{
    "bool", "int8", "uint8", "int16", "uint16", "int32", "uint32", "int64", "uint64", "float32", "float64", "float128",
};

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
            error("Invalid type name: " + type + "]");
        }
        type.pop_back();
        tmp += "[]";
    }
    if (baseNumericalData.contains(type)) {
        tmp += "f64";
    } else {
        if (tmp == "type") {
            error("Donot support type named \"type\"");
        }
        tmp += type;
    }
    return tmp;
}

} // namespace

namespace rulejit::xmlparser {

ruleset::RuleSetStructure XMLParser::parseXML(std::vector<char> srcXML) {
    // TODO: use string_view to hold
    using namespace rapidxml;
    using namespace ruleset;
    using namespace tools::mystr;
    using namespace std::literals;

    RuleSetStructure ret;
    ret.source = std::move(srcXML);
    if (ret.source.back()) {
        ret.source.push_back('\0');
    }

    // XML loading
    xml_document<> doc;
    doc.parse<parse_default>(ret.source.data());
    for (size_t i = 0; i < ret.source.size(); ++i) {
        if (ret.source[i] == '\n') {
            ret.lineBreaks.push_back(i);
        }
    }

    auto root = doc.first_node("RuleSet");

    if (auto it = root->first_attribute("version"); !it || it->value() != "1.0"sv) {
        error("Unsupported version of RuleSet");
    }
    // collect typedefine, add to typeOriginal amd RuleSetMetaInfo
    for (auto typeDef = root->first_node("TypeDefines")->first_node("TypeDefine"); typeDef;
         typeDef = typeDef->next_sibling("TypeDefine")) {
        std::string type = innerType(typeDef->first_attribute("type")->value());
        auto &tar = ret.typeDefines[type];
        for (auto member = typeDef->first_node("Variable"); member; member = member->next_sibling("Variable")) {
            auto memberName = member->first_attribute("name")->value();
            auto memberType = innerType(member->first_attribute("type")->value());
            tar.emplace_back(memberName, memberType);
        }
    }
    auto meta = root->first_node("MetaInfo");

    auto load = [&](const std::string &nodeName, std::vector<std::string> &target) {
        for (auto ele = meta->first_node(nodeName.data())->first_node("Param"); ele; ele = ele->next_sibling("Param")) {
            std::string name = ele->first_attribute("name")->value(),
                        type = innerType(ele->first_attribute("type")->value());
            if (ret.varType.contains(name)) {
                error("Input, Output and Cache variables should have different names");
            }
            ret.varType[name] = type;
            target.push_back(name);
            if (auto p = ele->first_node("Value"); p) {
                ret.intermediateValues.emplace_back(name, removeSpaceView({p->first_node("Expression")->value(),
                                                                           p->first_node("Expression")->value_size()}));
            }
            if (auto p = ele->first_node("InitValue"); p) {
                ret.initValues.emplace_back(name, removeSpaceView({p->first_node("Expression")->value(),
                                                                   p->first_node("Expression")->value_size()}));
            }
        }
    };

    load("Inputs", ret.inputVar);
    load("Caches", ret.cacheVar);
    load("Outputs", ret.outputVar);

    // generate subruleset defs
    for (auto subruleset = root->first_node("SubRuleSets")->first_node("SubRuleSet"); subruleset;
         subruleset = subruleset->next_sibling("SubRuleSet")) {

        ret.subRuleSets.push_back({});
        auto &subRuleSet = ret.subRuleSets.back();
        auto rules = subruleset->first_node("Rules");

        for (auto rule = rules->first_node("Rule"); rule; rule = rule->next_sibling("Rule")) {

            subRuleSet.atomRules.push_back({});
            auto &atom = subRuleSet.atomRules.back();
            atom.condition = removeSpaceView({rule->first_node("Condition")->first_node("Expression")->value(),
                                              rule->first_node("Condition")->first_node("Expression")->value_size()});

            for (auto cons = rule->first_node("Consequence")->first_node(); cons; cons = cons->next_sibling()) {

                std::string target = removeSpace(cons->first_node("Target")->value());
                auto baseName = target.substr(
                    0, std::ranges::find_if(target, [](char c) { return c == '.' || c == '['; }) - target.begin());

                if (cons->name() == "Assignment"sv) {
                    atom.consequences.push_back(
                        {std::move(target),
                         "assign",
                         {removeSpaceView({cons->first_node("Value")->first_node("Expression")->value(),
                                           cons->first_node("Value")->first_node("Expression")->value_size()})}});
                } else if (cons->name() == "ArrayOperation"sv || cons->name() == "Operation"sv) {
                    atom.consequences.push_back({std::move(target), cons->first_attribute("Operation")->value(), {}});
                    for (auto args = cons->first_node("Args"); args; args = args->next_sibling("Args")) {
                        atom.consequences.back().args.push_back(removeSpaceView(
                            {args->first_node("Expression")->value(), args->first_node("Expression")->value_size()}));
                    }
                } else {
                    error("Unknown Consequence type: "s + cons->name() + "");
                }
            }
        }
    }

    return ret;
}

} // namespace rulejit::xmlparser