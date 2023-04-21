/**
 * @file context.hpp
 * @author djw
 * @brief AST/Context
 * @date 2023-03-28
 *
 * @details Includes data structure to store context informations used in frontend or IR generation
 *
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-28</td><td>Initial version.</td></tr>
 * <tr><td>djw</td><td>2023-04-20</td><td>Add template support.</td></tr>
 * </table>
 */
#pragma once

#include <map>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "ast/ast.hpp"
#include "ast/type.hpp"
#include "defines/language.hpp"
#include "frontend/templateins.hpp"
#include "tools/seterror.hpp"

namespace rulejit {

/**
 * @brief global symbols and defines
 * todo: package support
 *
 */
struct ContextGlobal {
    /// @brief template function information
    struct TemplateFunctionInfo {
        std::map<std::vector<TypeInfo>, std::string> instantiationRealName;
        std::vector<std::string> paramNames;
        std::unique_ptr<FunctionDefAST> funcDef;
        /**
         * @brief instantiate this template function with given param type
         * 
         * @attention will not check or modify instantiationRealName
         * 
         * @param paramType type of each param
         * @return (func_ptr, matched), func_ptr is nullptr if instantiation failed; matched is map
         * from template param name to instantiated type
         */
        std::tuple<std::unique_ptr<FunctionDefAST>, std::map<std::string, TypeInfo>> instantiate(const std::vector<TypeInfo> &paramType) {
            std::set<std::string> templateParam{paramNames.begin(), paramNames.end()};
            std::map<std::string, TypeInfo> matched;
            auto &type = funcDef->funcType;
            auto cnt = type->getParamCount();
            if (cnt != paramType.size()) {
                return {nullptr, std::map<std::string, TypeInfo>{}};
            }
            for (size_t i = 0; i < cnt; ++i) {
                if (!type->getParamType(i).match(paramType[i], templateParam, matched)) {
                    return {nullptr, std::map<std::string, TypeInfo>{}};
                }
            }
            if (templateParam.size() != matched.size()) {
                return {nullptr, std::map<std::string, TypeInfo>{}};
            }
            // already matched, instantiate and return
            auto instantiation = funcDef->copy();
            TemplateInstantiator instantiator{TypeInfo::where(matched)};
            instantiation | instantiator;
            return {unique_cast<FunctionDefAST>(instantiation), std::move(matched)};
        }
    };

    /// @brief real function dependency graph
    std::unordered_map<std::string, std::set<std::string>> funcDependency;
    /// @brief real function name
    std::set<std::string> checkedFunc;

    /// @brief real function name -> function definition
    std::unordered_map<std::string, std::unique_ptr<FunctionDefAST>> realFuncDefinition;
    /// @brief extern function name -> type
    std::unordered_map<std::string, TypeInfo> externFuncDef;

    /// @brief used function name("add") -> real function name("func@0@2@add(f64,f64):f64")
    std::unordered_map<std::string, std::string> funcDef;
    /// @brief template name -> template function info
    std::unordered_map<std::string, TemplateFunctionInfo> templateFuncDef;

    /// @brief used function name("add") -> param type({"Vector3", "Vector3"}) -> real function
    /// name("func@0@2@add(Vector3,Vector3):Vector3")
    std::unordered_map<std::string, std::map<std::vector<TypeInfo>, std::string>> memberFuncDef;
    /// @brief template name -> template function info
    std::unordered_map<std::string, std::vector<TemplateFunctionInfo>> templateMemberFuncDef;

    /// @brief used function name("+") -> param type({"Vector3", "Vector3"}) -> real function
    /// name("func@0@2@+(Vector3,Vector3):Vector3")
    std::unordered_map<std::string, std::map<std::vector<TypeInfo>, std::string>> symbolicFuncDef;
    /// @brief template name -> template function info
    std::unordered_map<std::string, std::vector<TemplateFunctionInfo>> templateSymbolicFuncDef;

    /// @brief type alias name -> type name(may recursion)
    std::unordered_map<std::string, std::string> typeAlias;
    /// @brief type name -> defined type
    std::unordered_map<std::string, TypeInfo> typeDef;
};

/**
 * @brief symbol stack frame
 *
 */
struct ContextFrame {
    /// @brief var name / used function name -> type
    std::unordered_map<std::string, TypeInfo> varDef;

    /// @brief var name / used function name -> type
    std::unordered_map<std::string, std::tuple<TypeInfo, std::string>> constDef;

    // // var name -> real func closure type name list that capture this var
    // std::unordered_map<std::string, std::vector<std::string>> capturedInfo;
    // size_t scopeID = 0;
    // size_t subScopeCounter = 0;
};

/**
 * @brief context used in semantic analysis
 *
 * @details when meet var def: add {name, type} to varDef
 *
 * when meet type def: add {type name, type} to typeDef
 *
 * when meet func def:
 *
 * 1. register to "realFuncDefinition"
 * 2. add {used function name, real function name} to funcDef
 *
 * when meet member/infix func def:
 *
 * 1. register to "realFuncDefinition"
 * 2. add {used function name, {param type, real function name}} to symbolicFuncDef/memberFuncDef
 *
 */
struct ContextStack {

    /**
     * @brief Construct a new Context Stack object
     *
     */
    ContextStack() : scope({{}}), counter(0) {}
    ContextStack(const ContextStack &) = delete;
    ContextStack(ContextStack &&) = delete;
    ContextStack &operator=(const ContextStack &) = delete;
    ContextStack &operator=(ContextStack &&) = delete;

    /// @brief counter for generate unique name
    size_t counter;
    /// @brief global information
    ContextGlobal global;
    /// @brief scope stack
    std::vector<ContextFrame> scope;

    /**
     * @brief check if a symbol is unique in current scope(i.e. can defined as a new type/var/func)
     *
     * @param name
     * @return bool
     */
    bool isSymbolUnique(const std::string &name) {
        if (scope.back().varDef.contains(name) || scope.back().constDef.contains(name) ||
            (size() == 1 && global.templateFuncDef.contains(name))) {
            return false;
        }
        return true;
    }

    /**
     * @brief add a var definition
     *
     * @param name name of var
     * @param typeInfo type of var
     *
     * @return bool if success(failed when already defined)
     */
    [[nodiscard]] bool addVarDef(const std::string &name, const TypeInfo &typeInfo) {
        if (!isSymbolUnique(name)) {
            return false;
        }
        scope.back().varDef[name] = typeInfo;
        return true;
    };

    /**
     * @brief add a const definition
     *
     * @param name name of constant
     * @param typeInfo type of constant
     * @param value value of constant
     *
     * @return bool if success(failed when already defined)
     */
    [[nodiscard]] bool addConstDef(const std::string &name, const TypeInfo &typeInfo, const std::string &value) {
        if (!isSymbolUnique(name)) {
            return false;
        }
        scope.back().constDef[name] = {typeInfo, value};
        return true;
    };

    /**
     * @brief get real function type
     *
     * @param name real function name
     * @return const TypeInfo&
     */
    const TypeInfo &getRealFunctionType(const std::string &name) {
        if (auto it = global.realFuncDefinition.find(name); it != global.realFuncDefinition.end()) {
            return *(it->second->funcType);
        } else {
            error("cannot find function definition: " + name);
        }
    }

    /**
     * @brief get scope stack size
     *
     * @return size_t
     */
    size_t size() const { return scope.size(); }

    /**
     * @brief clear all context
     *
     */
    void clear() {
        counter = 0;
        global = {};
        scope = {{}};
    }

    /**
     * @brief get last scope
     *
     * @return ContextFrame&
     */
    ContextFrame &top() { return scope.back(); }

    /**
     * @brief generate unique name
     *
     * @param prefix name prefix
     * @param suffix name suffix
     * @return std::string generated name
     */
    std::string generateUniqueName(const std::string &prefix = "", const std::string &suffix = "") {
        std::string tmp = prefix + "@" + std::to_string(counter++) + "@" + suffix;
        return tmp;
    }

    /**
     * @brief push a scope to scope stack
     *
     * @return ContextFrame&
     */
    ContextFrame &push() {
        // auto tmp = scope.back().subScopeCounter++;
        scope.push_back({});
        // scope.back().subScopeCounter = 0;
        // scope.back().scopeID = tmp;
        return top();
    }

    /**
     * @brief pop the last scope from scope stack
     *
     * @return ContextFrame& last scope after pop
     */
    ContextFrame &pop() {
        scope.pop_back();
        return top();
    }

    /**
     * @brief function to seek var def in context stack
     *
     * @param s variable name
     * @return (find, type)
     */
    auto seekVarDef(const std::string &s) { return seek(&ContextFrame::varDef, s); }

    /**
     * @brief function to seek const def in context stack
     *
     * @param s variable name
     * @return (find, (type, value))
     */
    auto seekConstDef(const std::string &s) { return seek(&ContextFrame::constDef, s); }

  private:
    /**
     * @brief template function to seek def in context stack
     *
     * @tparam Item member pointer type
     * @tparam Index key type for unordered_map
     * @param p memner pointer
     * @param ind serached key
     * @param top current stack frame index, -1 for top frame
     * @return (find, value)
     */
    template <typename Item, typename Index> auto seek(Item p, const Index &ind, size_t top = size_t(-1)) {
        if (top == size_t(-1)) {
            top = scope.size() - 1;
        }
        auto it = (scope[top].*p).find(ind);
        if (it != (scope[top].*p).end()) {
            return std::tuple<bool, decltype(it->second) &>(true, it->second);
        } else if (top == 0) {
            static decltype(it->second) empty{};
            return std::tuple<bool, decltype(it->second) &>(false, empty);
        }
        return seek(p, ind, top - 1);
    }
};

} // namespace rulejit
