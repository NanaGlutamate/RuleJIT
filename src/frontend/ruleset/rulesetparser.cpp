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
 * <tr><td>djw</td><td>2023-04-24</td><td>Add more error info.</td></tr>
 * </table>
 */
#include <deque>

#include "rulesetparser.h"
#include "ast/escapedanalyzer.hpp"
#include "defines/marco.hpp"
#include "frontend/errorinfo.hpp"
#include "frontend/lexer.h"
#include "frontend/parser.h"
#include "frontend/semantic.hpp"
#include "rapidxml-1.13/rapidxml.hpp"
#include "tools/myassert.hpp"
#include "tools/seterror.hpp"
#include "tools/showmsg.hpp"
#include "tools/stringprocess.hpp"

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

/**
 * @brief pre-defines which will be process before ruleset xml,
 * can add some tool functions and types
 *
 */
const inline std::string preDefines = R"(
// base
extern func sin(a f64)->f64
extern func cos(a f64)->f64
extern func tan(a f64)->f64
extern func cot(a f64)->f64
extern func atan(a f64)->f64
extern func asin(a f64)->f64
extern func acos(a f64)->f64
extern func fabs(a f64)->f64
extern func exp(a f64)->f64
extern func floor(a f64)->f64
extern func sqrt(a f64)->f64
extern func pow(a f64, b f64)->f64
extern func atan2(a f64, b f64)->f64
extern func strEqual(a string, b string)->f64
func ==(a string, b string)->f64{strEqual(a, b)}
func abs(a f64)->f64 fabs(a)
const true f64 = 1.0
const false f64 = 0.0

func min(a f64, b f64)->f64 if(a>b) b else a
func max(a f64, b f64)->f64 if(a<b) b else a

// fuzzy logic
func trimf(x f64, a f64, b f64, c f64)->f64
    if(x < a)0
    else if(x < b) (x - a) / (b - a)
    else if(x < c) (c - x) / (c - b)
    else 0

func trapmf(x f64, a f64, b f64, c f64, d f64)->f64
    if(x < a)0
    else if(x < b) (x - a) / (b - a)
    else if(x < c) 1
    else if(x < d) (d - x) / (d - c)
    else 0

)";

} // namespace

// show more detailed info when lexer/parser/semantic error
RuleSetParseInfo RuleSetParser::readSource(const std::string &srcXML, ContextStack &context, RuleSetMetaInfo &data) {
    using namespace rapidxml;
    using namespace tools::mystr;

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
    std::string typeOriginal = "";
    // collect typedefine, add to typeOriginal amd RuleSetMetaInfo
    for (auto typeDef = root->first_node("TypeDefines")->first_node("TypeDefine"); typeDef;
         typeDef = typeDef->next_sibling("TypeDefine")) {
        std::string type = typeDef->first_attribute("type")->value();
        typeOriginal += "type " + type + " struct{";
        auto &tar = data.typeDefines[type];
        for (auto member = typeDef->first_node("Variable"); member; member = member->next_sibling("Variable")) {
            auto memberName = member->first_attribute("name")->value();
            auto memberType = member->first_attribute("type")->value();
            tar.emplace_back(memberName, memberType);
            typeOriginal += std::format("{} {};", memberName, innerType(memberType));
        }
        typeOriginal += "}\n";
    }
    auto meta = root->first_node("MetaInfo");

    // collect input/cache/output vars, and if element <Param> has sub element
    // named "InitValue", add assignment to initOriginal
    std::string initOriginal = "{\n";
    // when has sub element <Value>, add assignment to preprocessOriginal
    std::map<std::string, std::string> preprocessOriginal;

    auto load = [&](const std::string &nodeName, std::vector<std::string> &target) {
        for (auto ele = meta->first_node(nodeName.data())->first_node("Param"); ele; ele = ele->next_sibling("Param")) {
            std::string name = ele->first_attribute("name")->value(), type = ele->first_attribute("type")->value();
            if (data.varType.contains(name)) {
                error("Input, Output and Cache variables should have different names");
            }
            data.varType[name] = type;
            target.push_back(name);
            auto innertype = innerType(type);
            context.scope.back().varDef.emplace(name, innertype | lexer | TypeParser());
            if (auto p = ele->first_node("Value"); p) {
                // if contains <Value> node, add assignment to preprocessOriginal
                auto ip = ele->first_attribute("name")->value();
                // while (isspace(*ip)) {
                //     ip++;
                // }
                preprocessOriginal.emplace(std::string(ip),
                                           std::string("{") + removeSpace(p->first_node("Expression")->value()) + "}");
            }
            if (auto p = ele->first_node("InitValue"); p) {
                // TODO: add expression support?
                std::string tar = removeSpace(p->value());
                if (tar == "true") {
                    tar = "1.0";
                } else if (tar == "false") {
                    tar = "0.0";
                } else {
                    for (auto c : tar) {
                        if ((c < '0' || c > '9') && c != '.') {
                            error("InitValue should only be a literal number like 0, 0.0 or 3.14, "
                                  "no scientific notation or hex/oct/binary support");
                        }
                    }
                }
                // if contains <InitValue> node, add assignment to initOriginal
                initOriginal += std::string(ele->first_attribute("name")->value()) + "={" + tar + "};\n";
            }
        }
    };

    load("Inputs", data.inputVar);
    load("Caches", data.cacheVar);
    load("Outputs", data.outputVar);

    initOriginal += "}";
    try {
        // parse initOriginal, get returned real function name
        ret.preDefines = (typeOriginal + "\n" + preDefines + "\n" + initOriginal) | lexer | parser | semantic;
    } catch (std::logic_error &e) {
        auto info = genErrorInfo(semantic.getCallStack(), parser.AST2place, lexer.linePointer, lexer.beginPointer(),
                                 lexer.nextPointer());
        error(std::format("Error in preprocess pre-defines:\n\nwith information:\n\n{}\n\ndetails:\n\n{}", e.what(),
                          info.concatenateIdentifier()));
    }

    // topo-sort preprocessOriginal
    std::unordered_map<std::string, std::set<std::string>> valueDependency;
    std::vector<std::unique_ptr<ExprAST>> assignmentValue;
    // 1. collect dependency
    for (auto &&p : preprocessOriginal) {
        try {
            auto exprFuncName = p.second | lexer | parser | semantic;
            auto dependency = context.global.realFuncDefinition[exprFuncName]->returnValue | EscapedVarAnalyzer{};
            context.global.realFuncDefinition.erase(exprFuncName);
            my_assert(!valueDependency.contains(p.first), "assignment to a variable twice");
            if (dependency.contains(p.first)) {
                error("Self-dependent value is not allowed: " + p.first);
            }
            valueDependency.emplace(p.first, std::move(dependency));
        } catch (std::logic_error &e) {
            auto info = genErrorInfo(semantic.getCallStack(), parser.AST2place, lexer.linePointer, lexer.beginPointer(),
                                     lexer.nextPointer());
            error(std::format("Error in preprocess intermediate variable expression:\n\n    {} = {}\n\nwith "
                              "information:\n\n{}\n\ndetails:\n\n{}",
                              p.first, p.second, e.what(), info.concatenateIdentifier()));
        }
    }
    // 2. filter out false dependency
    for (auto &[name, dep] : valueDependency) {
        for (auto it = dep.begin(); it != dep.end();) {
            // is var and is intermediate
            if (data.varType.contains(*it) && valueDependency.contains(*it)) {
                it++;
            } else {
                it = dep.erase(it);
            }
        }
    }
    // 3. topo sort
    std::deque<std::string> openSet;
    std::vector<std::string> topoSorted;
    for (auto &&[k, v] : valueDependency) {
        if (v.empty()) {
            openSet.push_back(k);
        }
    }
    while (!openSet.empty()) {
        auto& cur = openSet.front();
        topoSorted.push_back(cur);
        for (auto &&[k, v] : valueDependency) {
            if (auto it = v.find(cur); it != v.end()) {
                v.erase(it);
                if (v.empty()) {
                    openSet.push_back(k);
                }
            }
        }
        openSet.pop_front();
    }
    if (topoSorted.size() != valueDependency.size()) {
        std::string errorMsg = "Cyclic dependency detected in preprocess intermediate variable assignment: \n";
        for (auto &&[k, v] : valueDependency) {
            auto it = std::find(topoSorted.begin(), topoSorted.end(), k);
            if (it == topoSorted.end()) {
                errorMsg += "\t" + k + " -> ";
                errorMsg += v | tools::mystr::join(", ");
                errorMsg += k + ";\n";
            }
        }
        error(errorMsg);
    }
    debugMsg("Topo sorted: " + (topoSorted | tools::mystr::join(", ")));
    std::string valueAssignment = "{\n";
    // parse preprocessOriginal, get returned real function name
    for (auto &&o_ : topoSorted) {
        auto &o = preprocessOriginal.find(o_).operator*();
        valueAssignment += (o.first + "=" + o.second + ";\n");
    }
    valueAssignment += "0}";
    data.modifiedValue.emplace_back(
        std::vector<std::set<std::string>>{std::set<std::string>{topoSorted.begin(), topoSorted.end()}});

    try {
        ret.preprocess.push_back(valueAssignment | lexer | parser | semantic);
    } catch (std::logic_error &e) {
        auto info = genErrorInfo(semantic.getCallStack(), parser.AST2place, lexer.linePointer, lexer.beginPointer(),
                                 lexer.nextPointer());
        error(std::format("Error in process intermediate variable assignment in XML\n\nwith "
                          "information:\n\n{}\n\ndetails:\n\n{}",
                          e.what(), info.concatenateIdentifier()));
    }

    // generate subruleset defs
    size_t id = 0;
    for (auto subruleset = root->first_node("SubRuleSets")->first_node("SubRuleSet"); subruleset;
         subruleset = subruleset->next_sibling("SubRuleSet"), id++) {

        data.modifiedValue.emplace_back();
        // std::vector<std::set<std::string>> singleModifiedValue;
        std::string expr = "{";
        size_t cnt = 0;
        auto rules = subruleset->first_node("Rules");
        for (auto rule = rules->first_node("Rule"); rule; rule = rule->next_sibling("Rule"), cnt++) {
            data.modifiedValue.back().emplace_back();
            expr += std::string("if({\n") +
                    removeSpace(rule->first_node("Condition")->first_node("Expression")->value()) + "\n}){\n";
            for (auto assign = rule->first_node("Consequence")->first_node(); assign; assign = assign->next_sibling()) {
                using namespace std::literals;
                std::string target = removeSpace(assign->first_node("Target")->value());
                auto baseName = target.substr(
                    0, std::ranges::find_if(target, [](char c) { return c == '.' || c == '['; }) - target.begin());
                data.modifiedValue.back().back().emplace(baseName);

                if (assign->name() == "Assignment"s) {
                    expr += target + "={" +
                            removeSpace(assign->first_node("Value")->first_node("Expression")->value()) + "};\n";
                } else if (assign->name() == "ArrayOperation"s) {
                    std::string value;
                    if (auto valueNode = assign->first_node("Args"); valueNode) {
                        value = removeSpace(valueNode->first_node("Expression")->value());
                    }
                    expr +=
                        std::format("{}.{}({});", target, removeSpace(assign->first_node("Operation")->value()), value);
                } else {
                    error("Unknown Consequence type: "s + assign->name() + "");
                }

            }
            expr += "\n" + std::to_string(cnt) + "\n}else ";
        }
        expr += "{-1}}";
        try {
            auto astName = std::unique_ptr<rulejit::ExprAST>(expr | lexer | parser) | semantic;
            ret.subRuleSets.push_back(astName);
        } catch (std::logic_error &e) {
            auto info = genErrorInfo(semantic.getCallStack(), parser.AST2place, lexer.linePointer, lexer.beginPointer(),
                                     lexer.nextPointer());
            error(std::format("Error in process sub ruleset No.{}(zero-based) in XML\n\nwith "
                              "information:\n\n{}\n\ndetails:\n\n{}",
                              id, e.what(), info.concatenateIdentifier()));
        }
#ifdef __RULEJIT_DEBUG_IN_RUNTIME
        std::map<ExprAST *, std::string> table;
        for (auto [p, s] : parser.AST2place) {
            table.emplace(p, s);
        }
        ret.debugInfo.push_back(std::move(table));
#endif // __RULEJIT_DEBUG_IN_RUNTIME
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