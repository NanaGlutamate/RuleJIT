#pragma once

#pragma once

#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "ast/type.hpp"
#include "defines/language.hpp"

namespace rulejit {

struct SymbolTable {
    std::string getFuncOverloadSymbol() {}
};

struct ContextGlobal {
    // real function name -> (capture type name, function definition)
    std::map<std::string, std::tuple<std::string, std::unique_ptr<FunctionDefAST>>> realFuncDefinition;
    std::map<std::string, TypeInfo> externTypeDef;
    std::map<std::string, TypeInfo> externFuncDef;
    std::map<std::string, size_t> overLoadNumberRegister;
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
    // TODO: std::set<std::string> escapedVar;
    size_t scopeID;
    size_t subScopeCounter;
};

// persistence stack
struct ContextStack {
    ContextGlobal global;
    std::vector<ContextFrame> stackFrame;
    std::vector<size_t> preFrame;
    std::vector<size_t> stackTop;
    // ContextStack(){
    //     std::map<std::string, int> tmp;
    //     for(auto&& i : reloadableBuildInUnary){
    //         tmp.emplace(i, 0);
    //     }
    //     stack.push_back(ContextFrame{
    //         {},
    //         {},
    //         {},
    //         buildInFunc,
    //         reloadableBuildInInfix,
    //         tmp,
    //     });
    // }
    ContextStack() : stackFrame({}), preFrame({size_t(-1)}), stackTop({0}) {
        stackFrame.back().scopeID = 0;
        stackFrame.back().subScopeCounter = 0;
    }
    std::string genUniqueName() {
        std::string name = "unnamed_";
        size_t frameNow = stackTop.back();
        while (frameNow != size_t(-1)) {
            // ensure unique in other scope
            name += std::to_string(stackFrame[frameNow].scopeID) + "_";
            frameNow = preFrame[frameNow];
        }
        static size_t specifier = 0;
        while (true) {
            // ensure unique in this scope
            std::string tmp = name + std::to_string(specifier);
            if (top().funcDef.contains(tmp) || top().infixFuncDef.contains(tmp) || top().memberFuncDef.contains(tmp) ||
                top().typeAlias.contains(tmp) || top().typeDef.contains(tmp) || top().varDef.contains(tmp)) {
                ++specifier;
            } else {
                return tmp;
            }
        }
    }
    std::string genRealFunctionName(const TypeInfo &type) {
        my_assert(type.isFunctionType());
        std::string ret = "func@";
        size_t frameNow = stackTop.back();
        while (frameNow != size_t(-1)) {
            ret += std::to_string(stackFrame[frameNow].scopeID) + "@";
            frameNow = preFrame[frameNow];
        }
        for (int i = 0; i < type.idents.size(); ++i) {
            ret += type.idents[i];
        }
        return ret;
    }
    ContextFrame &top() { return stackFrame[stackTop.back()]; }
    ContextFrame &push() {
        stackFrame.push_back({});
        preFrame.push_back(stackTop.back());
        stackTop.push_back(stackFrame.size() - 1);
        stackFrame.back().subScopeCounter = 0;
        stackFrame.back().scopeID = stackFrame[preFrame[stackTop.back()]].subScopeCounter++;
        return top();
    }
    ContextFrame &pop() {
        stackTop.push_back(preFrame[stackTop.back()]);
        return top();
    }
    template <typename Item, typename Index>
    //! @return (find, escaped, value)
    decltype(auto) seek(Item p, const Index &ind, size_t top = size_t(-1), bool escaped = true) {
        if (top == size_t(-1)) {
            top = stackTop.back();
            escaped = false;
        }
        auto it = (stackFrame[top].*p).find(ind);
        if (it != (stackFrame[top].*p).end()) {
            return std::tuple<bool, bool, decltype(it->second) &>(true, escaped, it->second);
        } else if (top == 0) {
            static decltype(it->second) empty{};
            return std::tuple<bool, bool, decltype(it->second) &>(false, escaped, empty);
        }
        return seek(p, ind, preFrame[top]);
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
    decltype(auto) seekTypeDef(const std::string s) {
        // auto tmp = seekTypeAlias(s);
        // while (std::get<0>(tmp)) {
        //     s = std::get<2>(tmp);
        //     tmp = seekTypeAlias(s);
        // }
        return seek(&ContextFrame::typeDef, s);
    }
    //! @return (find, escaped, value)
    decltype(auto) seekVarDef(const std::string &s) { return seek(&ContextFrame::varDef, s); }

    // std::tuple<bool, TypeInfo> seekTypeAlias(const std::string& s, size_t layer = size_t(-1)){
    //     if(layer == size_t(-1)){
    //         layer = stack.size()-1;
    //     }
    //     if(auto it = stack[layer].typeAlias.find(s); it != stack[layer].typeAlias.end()){
    //         return {true, it->second};
    //     }
    //     if(layer == 0){
    //         return {false, NoInstanceType};
    //     }
    //     return seekTypeAlias(s, layer-1);
    // }
    // bool seekUnaryOp(const std::string& s, size_t layer = size_t(-1)){
    //     if(layer == size_t(-1)){
    //         layer = stack.size()-1;
    //     }
    //     if(auto it = stack[layer].unaryOp.find(s); it != stack[layer].unaryOp.end()){
    //         return true;
    //     }
    //     if(layer == 0){
    //         return false;
    //     }
    //     return seekUnaryOp(s, layer-1);
    // }
    // std::tuple<bool, Priority> seekInfixOp(const std::string& s, size_t layer = size_t(-1)){
    //     if(layer == size_t(-1)){
    //         layer = stack.size()-1;
    //     }
    //     if(auto it = stack[layer].infixOp.find(s); it != stack[layer].infixOp.end()){
    //         return {true, it->second};
    //     }
    //     if(layer == 0){
    //         return {false, 0};
    //     }
    //     return seekInfixOp(s, layer-1);
    // }
    // std::tuple<bool, const TypeInfo&> seekFuncDef(const std::tuple<std::string, std::vector<TypeInfo>>& param, size_t
    // layer = size_t(-1)){
    //     if(layer == size_t(-1)){
    //         layer = stack.size()-1;
    //     }
    //     if(auto it = stack[layer].funcDef.find(param); it != stack[layer].funcDef.end()){
    //         return {true, it->second};
    //     }
    //     if(layer == 0){
    //         return {false, NoInstanceType};
    //     }
    //     return seekFuncDef(param, layer-1);
    // }
    // std::tuple<bool, const TypeInfo&> seekVarDef(const std::string& s, size_t layer = size_t(-1)){
    //     if(layer == size_t(-1)){
    //         layer = stack.size()-1;
    //     }
    //     if(auto it = stack[layer].varDef.find(s); it != stack[layer].varDef.end()){
    //         return {true, it->second};
    //     }
    //     if(layer == 0){
    //         return {false, NoInstanceType};
    //     }
    //     return seekVarDef(s, layer-1);
    // }
    // std::tuple<bool, const TypeInfo&> seekTypeDef(const std::string& s, size_t layer = size_t(-1)){
    //     if(layer == size_t(-1)){
    //         layer = stack.size()-1;
    //     }
    //     if(auto it = stack[layer].typeDef.find(s); it != stack[layer].typeDef.end()){
    //         return {true, it->second};
    //     }
    //     if(layer == 0){
    //         return {false, NoInstanceType};
    //     }
    //     return seekTypeDef(s, layer-1);
    // }
};

} // namespace rulejit
