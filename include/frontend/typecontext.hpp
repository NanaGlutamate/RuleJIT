#pragma once

#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "ast/type.hpp"
#include "defines/language.hpp"

namespace rulejit{

struct ContextGlobal {
    std::map<std::string, std::vector<size_t>> memberFuncRegister;
    std::map<std::string, size_t> overLoadNumberRegister;
    std::map<std::string, std::map<size_t, TypeInfo>> funcDef;
};

struct ContextFrame {
    std::map<std::string, TypeInfo> typeAlias;
    std::map<std::string, TypeInfo> typeDef;
    std::map<std::string, TypeInfo> varDef;
    std::map<std::string, std::vector<size_t>> visibleFunc;
};

struct ContextStack {
    std::vector<ContextFrame> stack;
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
    ContextFrame& top(){
        return stack[stack.size()-1];
    }
    ContextFrame& push(){
        stack.push_back({});
        return top();
    }
    ContextFrame& pop(){
        stack.pop_back();
        return top();
    }
    template<typename Item, typename Index>
    decltype(auto) seek(Item p, const Index& ind, size_t layer = size_t(-1)){
        if(layer == size_t(-1)){
            layer = stack.size()-1;
        }
        auto it = (stack[layer].*p).find(ind);
        if(it != (stack[layer].*p).end()){
            return std::tuple<bool, const decltype(it->second)&>(true, it->second);
        }else if(layer == 0){
            // static const decltype(it->second) empty{};
            return std::tuple<bool, const decltype(it->second)&>(false, {});
        }
        return seek(p, ind, layer-1);
    }
    decltype(auto) seekTypeAlias(const std::string& s){
        return seek(&ContextFrame::typeAlias, s);
    }
    decltype(auto) seekTypeDef(const std::string& s){
        return seek(&ContextFrame::typeDef, s);
    }
    decltype(auto) seekVarDef(const std::string& s){
        return seek(&ContextFrame::varDef, s);
    }
    decltype(auto) seekFuncDef(const std::string& s){
        return seek(&ContextFrame::visibleFunc, s);
    }

    // std::tuple<bool, TypeInfo> seekTypeAlias(const std::string& s, size_t layer = size_t(-1)){
    //     if(layer == size_t(-1)){
    //         layer = stack.size()-1;
    //     }
    //     if(auto it = stack[layer].typeAlias.find(s); it != stack[layer].typeAlias.end()){
    //         return {true, it->second};
    //     }
    //     if(layer == 0){
    //         return {false, voidType};
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
    // std::tuple<bool, const TypeInfo&> seekFuncDef(const std::tuple<std::string, std::vector<TypeInfo>>& param, size_t layer = size_t(-1)){
    //     if(layer == size_t(-1)){
    //         layer = stack.size()-1;
    //     }
    //     if(auto it = stack[layer].funcDef.find(param); it != stack[layer].funcDef.end()){
    //         return {true, it->second};
    //     }
    //     if(layer == 0){
    //         return {false, voidType};
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
    //         return {false, voidType};
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
    //         return {false, voidType};
    //     }
    //     return seekTypeDef(s, layer-1);
    // }
};

} // namespace rulejit
