#pragma once

#pragma once

#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "ast/ast.hpp"
#include "ast/type.hpp"
#include "defines/language.hpp"

namespace rulejit {

struct SymbolTable {
    std::string getFuncOverloadSymbol() {}
};

struct ContextGlobal {
    // real function name
    std::set<std::string> UncheckedFunc;
    // real function name -> function definition
    std::map<std::string, std::unique_ptr<FunctionDefAST>> realFuncDefinition;
    std::map<std::string, TypeInfo> externFuncDef;
    // used function name("add") -> real function name("func@0@2@add(f64,f64):f64")
    std::map<std::string, std::string> funcDef;
    // used function name("+") -> param type({"Vector3", "Vector3"}) -> real function
    //     name("func@0@2@+(Vector3,Vector3):Vector3")
    std::map<std::string, std::map<std::vector<TypeInfo>, std::string>> symbolicFuncDef;
    // used function name("add") -> param type({"Vector3", "Vector3"}) -> real function
    //     name("func@0@2@add(Vector3,Vector3):Vector3")
    std::map<std::string, std::map<std::vector<TypeInfo>, std::string>> memberFuncDef;
    // type alias name -> type name(may recursion)
    std::map<std::string, std::string> typeAlias;
    // type name -> defined type
    std::map<std::string, TypeInfo> typeDef;
};

// when meet var def: add {name, type} to varDef
// when meet type def: add {type name, type} to typeDef
// when meet type alias: 1. if alias to a unnamed type, name it and define it
//                       2. add {type alias name, typename} to typeAlias
// when meet func def: 1. register to "realFuncDefinition"
//                     2. add {used function name, real function name} to funcDef
//                     3. add {used function name, func type} to vardef
// when meet member/infix func def: 1. register to "realFuncDefinition"
//                                  2. add {used function name, {param type, real function name}} to
//                                     symbolicFuncDef/memberFuncDef
// only when all def in a scope processed can sub scope to be processed.
struct ContextFrame {
    // var name / used function name -> type
    std::map<std::string, TypeInfo> varDef;
    // // var name / used function name -> type
    // std::map<std::string, TypeInfo> literalVarDef;
    // std::set<std::string> capturedSymbol;
    size_t scopeID = 0;
    size_t subScopeCounter = 0;
};

// stack
struct ContextStack {
    size_t counter;
    ContextGlobal global;
    std::vector<ContextFrame> stackFrame;
    ContextStack() : stackFrame({{}}), counter(0) {}
    ContextStack(const ContextStack &) = delete;
    ContextStack(ContextStack &&) = delete;
    ContextStack &operator=(const ContextStack &) = delete;
    ContextStack &operator=(ContextStack &&) = delete;
    const TypeInfo &getRealFunctionType(const std::string &name) {
        if (auto it = global.realFuncDefinition.find(name); it != global.realFuncDefinition.end()) {
            return *(it->second->funcType);
        } else {
            throw std::logic_error("cannot find function definition: " + name);
        }
    }
    size_t size() const {
        return stackFrame.size();
    }
    void clear() {
        counter = 0;
        global = {};
        stackFrame = {{}};
    }
    ContextFrame &top() { return stackFrame.back(); }
    std::string generateUniqueName(const std::string &prefix = "", const std::string &suffix = "") {
        std::string tmp = prefix + "_" + std::to_string(counter++) + "_" + suffix;
        return tmp;
    }
    ContextFrame &push() {
        auto tmp = stackFrame.back().subScopeCounter++;
        stackFrame.push_back({});
        stackFrame.back().subScopeCounter = 0;
        stackFrame.back().scopeID = tmp;
        return top();
    }
    ContextFrame &pop() {
        stackFrame.pop_back();
        return top();
    }
    template <typename Item, typename Index>
    //! @return (find, escaped, value)
    decltype(auto) seek(Item p, const Index &ind, size_t top = size_t(-1)) {
        if (top == size_t(-1)) {
            top = stackFrame.size() - 1;
        }
        auto it = (stackFrame[top].*p).find(ind);
        if (it != (stackFrame[top].*p).end()) {
            return std::tuple<bool, decltype(it->second) &>(true, it->second);
        } else if (top == 0) {
            static decltype(it->second) empty{};
            return std::tuple<bool, decltype(it->second) &>(false, empty);
        }
        return seek(p, ind, top - 1);
    }
    // //! @return (find, escaped, value)
    // decltype(auto) seekFuncDef(const std::string &s) { return seek(&ContextFrame::funcDef, s); }
    // //! @return (find, escaped, value)
    // decltype(auto) seekInfixFuncDef(const std::string &s) { return seek(&ContextFrame::symbolicFuncDef, s); }
    // //! @return (find, escaped, value)
    // decltype(auto) seekMemberFuncDef(const std::string &s) { return seek(&ContextFrame::memberFuncDef, s); }
    // //! @return (find, escaped, value)
    // decltype(auto) seekTypeAlias(const std::string &s) { return seek(&ContextFrame::typeAlias, s); }
    // //! @return (find, escaped, value)
    // decltype(auto) seekTypeDef(const std::string s) { return seek(&ContextFrame::typeDef, s); }
    //! @return (find, escaped, value)
    decltype(auto) seekVarDef(const std::string &s) { return seek(&ContextFrame::varDef, s); }
};

} // namespace rulejit
