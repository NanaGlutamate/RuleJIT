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
 * </table>
 */
#pragma once

#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "ast/ast.hpp"
#include "ast/type.hpp"
#include "defines/language.hpp"
#include "tools/seterror.hpp"

namespace rulejit {

/**
 * @brief global symbols and defines
 *
 */
struct ContextGlobal {
    /// @brief real function dependency graph
    std::map<std::string, std::set<std::string>> funcDependency;
    /// @brief real function name
    std::set<std::string> checkedFunc;
    /// @brief real function name -> function definition
    std::map<std::string, std::unique_ptr<FunctionDefAST>> realFuncDefinition;
    /// @brief real function name -> function definition
    std::map<std::string, TypeInfo> externFuncDef;
    /// @brief used function name("add") -> real function name("func@0@2@add(f64,f64):f64")
    std::map<std::string, std::string> funcDef;
    /**
     *  @brief used function name("+") -> param type({"Vector3", "Vector3"}) -> real function
     *  name("func@0@2@+(Vector3,Vector3):Vector3")
     */
    std::map<std::string, std::map<std::vector<TypeInfo>, std::string>> symbolicFuncDef;
    /**
     * @brief used function name("add") -> param type({"Vector3", "Vector3"}) -> real function
     * name("func@0@2@add(Vector3,Vector3):Vector3")
     *
     */
    std::map<std::string, std::map<std::vector<TypeInfo>, std::string>> memberFuncDef;
    /// @brief type alias name -> type name(may recursion)
    std::map<std::string, std::string> typeAlias;
    /// @brief type name -> defined type
    std::map<std::string, TypeInfo> typeDef;
};

/**
 * @brief symbol stack frame
 *
 */
struct ContextFrame {
    /// @brief var name / used function name -> type
    std::map<std::string, TypeInfo> varDef;
    // // var name -> real func closure type name list that capture this var
    // std::map<std::string, std::vector<std::string>> capturedInfo;
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

    std::string getRealFunctionNameOfNormalFunctionWithHint(const std::string &name, const TypeInfo &hint){}

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
        std::string tmp = prefix + "_" + std::to_string(counter++) + "_" + suffix;
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
     * @return (find, escaped, value)
     */
    auto seekVarDef(const std::string &s) { return seek(&ContextFrame::varDef, s); }

  private:
    /**
     * @brief template function to seek def in context stack
     *
     * @tparam Item member pointer type
     * @tparam Index key type for map
     * @param p memner pointer
     * @param ind serached key
     * @param top current stack frame index, -1 for top frame
     * @return (find, escaped, value)
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
