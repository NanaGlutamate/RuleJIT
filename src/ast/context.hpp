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
    // real function name -> (capture type name, function definition)
    std::map<std::string, std::tuple<std::string, std::unique_ptr<FunctionDefAST>>> realFuncDefinition;
    std::map<std::string, TypeInfo> externFuncDef;
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
//                                     infixFuncDef/memberFuncDef
// only when all def in a scope processed can sub scope to be processed.
struct ContextFrame {
    // used function name("add") -> real function name("func@0@2@add(f64,f64):f64")
    std::map<std::string, std::string> funcDef;
    // used function name("+") -> param type({"Vector3", "Vector3"}) -> real function
    //     name("func@0@2@+(Vector3,Vector3):Vector3")
    std::map<std::string, std::map<std::vector<TypeInfo>, std::string>> infixFuncDef;
    // used function name("add") -> param type({"Vector3", "Vector3"}) -> real function
    //     name("func@0@2@add(Vector3,Vector3):Vector3")
    std::map<std::string, std::map<std::vector<TypeInfo>, std::string>> memberFuncDef;
    // type alias name -> type name(may recursion)
    std::map<std::string, std::string> typeAlias;
    // type name -> defined type
    std::map<std::string, TypeInfo> typeDef;
    // var name / used function name -> type
    std::map<std::string, TypeInfo> varDef;
    // // var name / used function name -> type
    // std::map<std::string, TypeInfo> literalVarDef;
    // std::set<std::string> capturedSymbol;
    size_t scopeID;
    size_t subScopeCounter;
};

// stack
struct ContextStack {
    ContextGlobal global;
    std::vector<ContextFrame> stackFrame;
    ContextStack() : stackFrame({}){}
    ContextStack(const ContextStack &) = delete;
    ContextFrame &top() { return stackFrame.back(); }
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
    //! @return (find, escaped, value)
    decltype(auto) seekFuncDef(const std::string &s) { return seek(&ContextFrame::funcDef, s); }
    //! @return (find, escaped, value)
    decltype(auto) seekInfixFuncDef(const std::string &s) { return seek(&ContextFrame::infixFuncDef, s); }
    //! @return (find, escaped, value)
    decltype(auto) seekMemberFuncDef(const std::string &s) { return seek(&ContextFrame::memberFuncDef, s); }
    //! @return (find, escaped, value)
    decltype(auto) seekTypeAlias(const std::string &s) { return seek(&ContextFrame::typeAlias, s); }
    //! @return (find, escaped, value)
    decltype(auto) seekTypeDef(const std::string s) { return seek(&ContextFrame::typeDef, s); }
    //! @return (find, escaped, value)
    decltype(auto) seekVarDef(const std::string &s) { return seek(&ContextFrame::varDef, s); }
};

} // namespace rulejit
