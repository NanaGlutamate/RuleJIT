/**
 * @file rulesetparser.cpp
 * @author djw
 * @brief FrontEnd/Ruleset
 * @date 2023-03-29
 *
 * @details Includes common codes for parsing ruleset.
 *
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-29</td><td>Initial version.</td></tr>
 * </table>
 */
#include "rulesetparser.h"
#include "frontend/lexer.h"
#include "frontend/parser.h"
#include "frontend/semantic.hpp"
#include "rapidxml-1.13/rapidxml.hpp"
#include "tools/myassert.hpp"
#include "tools/seterror.hpp"

namespace {

using namespace rulejit;
using namespace rulejit::rulesetxml;

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
    if (baseData.contains(type)) {
        tmp += "f64";
    } else {
        tmp += type;
    }
    return tmp;
}

/**
 * @brief pre-defines which will be process before ruleset xml,
 * can add some tool functions and types
 *
 */
const inline std::string preDefines = R"(
extern func sin(a f64):f64
extern func cos(a f64):f64
extern func tan(a f64):f64
extern func cot(a f64):f64
extern func atan(a f64):f64
extern func asin(a f64):f64
extern func acos(a f64):f64
extern func fabs(a f64):f64
extern func exp(a f64):f64
extern func abs(a f64):f64
extern func floor(a f64):f64
extern func sqrt(a f64):f64
extern func pow(a f64, b f64):f64
extern func atan2(a f64, b f64):f64
)";

} // namespace

RuleSetParseInfo RuleSetParser::readSource(const std::string &srcXML, ContextStack &context, RuleSetMetaInfo &data) {
    using namespace rapidxml;

    static ExpressionLexer lexer;
    static ExpressionParser parser;

    ExpressionSemantic semantic(context);

    my_assert(context.size() == 1);

    RuleSetParseInfo ret;

    // XML loading
    xml_document<> doc;
    CStyleString s(srcXML);
    doc.parse<parse_default>(s.s);

    auto root = doc.first_node("RuleSet");

    if (auto it = root->first_attribute("version"); !it || it->value() != std::string("1.0")) {
        error("Unsupported version of RuleSet");
    }
    for (auto typeDef = root->first_node("TypeDefines")->first_node("TypeDefine"); typeDef;
         typeDef = typeDef->next_sibling("TypeDefine")) {
        std::string type = typeDef->first_attribute("type")->value();
        auto &tar = data.typeDefines[type];
        for (auto member = typeDef->first_node("Variable"); member; member = member->next_sibling("Variable")) {
            tar.emplace_back(member->first_attribute("name")->value(), member->first_attribute("type")->value());
        }
    }
    auto meta = root->first_node("MetaInfo");

    // collect input/cache/output vars, and if element <Param> has sub element
    // named "InitValue", add assignment to initOriginal
    std::string initOriginal = "{";
    // when has sub element named "Value", add assignment to preprocessOriginal
    std::string preprocessOriginal = "{";

    auto load = [&](const std::string &nodeName, std::vector<std::string> &target) {
        for (auto ele = meta->first_node(nodeName.data())->first_node("Param"); ele;
             ele = ele->next_sibling("Param")) {
            std::string name = ele->first_attribute("name")->value(),
                        type = ele->first_attribute("type")->value();
            if (data.varType.contains(name)) {
                error("Input, Output and Cache variables should have different names");
            }
            target.push_back(name);
            data.varType[name] = type;
            context.scope.back().varDef.emplace(name, innerType(type) | lexer | TypeParser());
            if (auto p = ele->first_node("Value"); p) {
                // if contains <Value> node, add assignment to preprocessOriginal
                preprocessOriginal += std::string(ele->first_attribute("name")->value()) + "={" +
                                      p->first_node("Expression")->value() + "};";
            }
            if (auto p = ele->first_node("InitValue"); p) {
                // TODO: add expression support?
                std::string tar = p->value();
                if (tar == "true") {
                    tar = "1.0";
                } else if (tar == "false") {
                    tar = "0.0";
                } else {
                    for (auto c : tar) {
                        if ((c < '0' || c > '9') && c != '.') {
                            error("InitValue should only be a number like 0, 0.0 or 3.14, no scientific notation "
                                  "or hex/oct/binary support");
                        }
                    }
                }
                // if contains <InitValue> node, add assignment to initOriginal
                initOriginal += std::string(ele->first_attribute("name")->value()) + "={" + tar + "};";
            }
        }
    };

    load("Inputs", data.inputVar);
    load("Caches", data.cacheVar);
    load("Outputs", data.outputVar);

    initOriginal += "}";
    preprocessOriginal += "}";
    // parse initOriginal, get returned real function name
    ret.preDefines = (preDefines + "\n" + initOriginal) | lexer | parser | semantic;
    // parse preprocessOriginal, get returned real function name
    ret.preprocess = std::unique_ptr<rulejit::ExprAST>(preprocessOriginal | lexer | parser) | semantic;

    // generate subruleset defs
    size_t id = 0;
    for (auto subruleset = root->first_node("SubRuleSets")->first_node("SubRuleSet"); subruleset;
         subruleset = subruleset->next_sibling("SubRuleSet"), id++) {

        std::string expr = "{";
        auto rules = subruleset->first_node("Rules");
        size_t act = 0;
        for (auto rule = rules->first_node("Rule"); rule; rule = rule->next_sibling("Rule"), act++) {
            expr += std::string("if({") + rule->first_node("Condition")->first_node("Expression")->value() + "}){";
            // TODO: add direct expression support
            for (auto assign = rule->first_node("Consequence")->first_node("Assignment"); assign;
                 assign = assign->next_sibling("Assignment")) {
                expr += std::string(assign->first_node("Target")->value()) + "={" +
                        assign->first_node("Value")->first_node("Expression")->value() + "};";
            }
            expr += std::to_string(act) + "}else ";
        }
        expr += "{-1}}";
        auto astName = std::unique_ptr<rulejit::ExprAST>(expr | lexer | parser) | semantic;
        ret.subRuleSets.push_back(astName);
    }

    // check all defined function
    for (auto &&[name, _] : context.global.realFuncDefinition) {
        if (context.global.checkedFunc.contains(name)) {
            continue;
        }
        semantic.checkFunction(name);
    }

    return ret;
}