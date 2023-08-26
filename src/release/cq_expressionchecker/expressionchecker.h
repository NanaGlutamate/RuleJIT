#include <format>
#include <string>
#include <tuple>

#include "frontend/errorinfo.hpp"
#include "frontend/lexer.h"
#include "frontend/parser.h"
#include "frontend/ruleset/rulesetparser.h"
#include "frontend/semantic.hpp"
#include "backend/cq/cqrulesetengine.h"

struct ExpressionChecker {
    ExpressionChecker() : semantic(context) {}

    /**
     * @brief reset all context, including type definition and variable definition
     * 
     */
    void reset(){
        context.global.typeDef.clear();
        context.top().varDef.clear();
    }

    /**
     * @brief add a type definition to expression context
     *
     * @example usage:
     * to add a type named "Vector3" with members "x", "y", "z" and their types are all "float64",
     * call addTypeDef("Vector3", {{"x", "float64"}, {"y", "float64"}, {"z", "float64"}});
     *
     * @param typeName name of defined complex type
     * @param typeInfo member name and type of defined complex type
     */
    void addTypeDef(const std::string &typeName, const std::vector<std::tuple<std::string, std::string>> &typeInfo) {
        using namespace std;
        using namespace rulejit;
        vector<tuple<string, TypeInfo>> typeDef;
        for (auto &&[name, type] : typeInfo) {
            typeDef.emplace_back(name, make_type(innerType(type)));
        }
        context.global.typeDef[typeName] = move(typeDef);
    }

    /**
     * @brief remove a type definition from expression context
     * 
     * @param typeName name of defined complex type
     */
    void removeTypeDef(const std::string &typeName) {
        using namespace std;
        using namespace rulejit;
        context.global.typeDef.erase(typeName);
    }

    /**
     * @brief add a variable definition to expression context
     *
     * @example usage:
     * to add a variable named "a" with type "float64", call addVarDef("a", "float64");
     *
     * @param varName name of defined variable
     * @param varType type of defined variable
     */
    void addVarDef(const std::string &varName, const std::string &varType) {
        using namespace std;
        using namespace rulejit;
        context.top().varDef[varName] = make_type(innerType(varType));
    }

    /**
     * @brief remove a variable definition from expression context
     * 
     * @param varName name of defined variable
     */
    void removeVarDef(const std::string &varName) {
        using namespace std;
        using namespace rulejit;
        context.top().varDef.erase(varName);
    }

    /**
     * @brief check if a expression is legal
     *
     * @param expression expression to check
     * @return std::tuple<std::string, bool> (error_message, is_error)
     */
    std::tuple<std::string, bool> checkExpression(const std::string &expression) {
        using namespace std;
        using namespace rulejit;
        static ExpressionLexer lexer;
        static ExpressionParser parser;
        auto expr = ("{\n" + expression + "\n}");
        try {
            auto name = expr | lexer | parser | semantic;
            context.global.realFuncDefinition.clear();
        } catch (logic_error &e) {
            auto info = genErrorInfo(semantic.getCallStack(), parser.AST2place, lexer.linePointer, lexer.beginPointer(),
                                     lexer.nextPointer());
            return {
                std::format("Error:\n{}\nLocation:\n\n{}", e.what(), info.concatenateIdentifier()),
                true};
        }
        return {"", false};
    }

    /**
     * @brief check if a xml file is legal
     *
     * @param xml xml to check
     * @return std::tuple<std::string, bool> (error_message, is_error)
     */
    std::tuple<std::string, bool> checkXML(const std::string &xml) {
        using namespace std;
        using namespace rulejit;
        cq::RuleSetEngine engine;
        try{
            engine.buildFromSource(xml);
            context.global.realFuncDefinition.clear();
        }catch(logic_error &e){
            return {e.what(), true};
        }
        return {"", false};
    }

  private:
    /**
     * @brief transform cpp style type string to inner type string
     *
     * @param type cpp style type string
     * @return std::string inner type string
     */
    static std::string innerType(std::string type) {
        std::string tmp;
        while (type.back() == ']') {
            type.pop_back();
            if (type.back() != '[') {
                error("Invalid type name: " + type + "]");
            }
            type.pop_back();
            tmp += "[]";
        }
        if (rulejit::ruleset::baseNumericalData.contains(type)) {
            tmp += "f64";
        } else {
            if (tmp == "type") {
                error("Donot support type named \"type\"");
            }
            tmp += type;
        }
        return tmp;
    }
    rulejit::ContextStack context;
    rulejit::ExpressionSemantic semantic;
};