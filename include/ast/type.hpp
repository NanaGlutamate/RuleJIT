#pragma once

#include <algorithm>
#include <map>
#include <memory>
#include <ranges>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace rulejit {

std::set<std::string> buildInType{
    // "i64",
    // "u64",
    "f64",
    "string",
};

struct TypeInfo {
    virtual bool isArray() { return false; }
    virtual bool isFunction() { return false; }
    virtual bool isIdent() { return false; }
    virtual TypeInfo *memberType(const std::string &ident) { return nullptr; }
    // virtual bool isLegal(){return false;}
    virtual ~TypeInfo() = default;
};

// a
struct TypeIdent : public TypeInfo {
    std::string name;
    virtual bool isIdent() { return true; }
};

// // [4]a | TODO: [5,5]a
// struct ArrayType : public TypeInfo{
//     size_t len;
//     std::unique_ptr<TypeInfo> baseType;
//     bool isArray(){return true;}
// };

// []a
struct SliceType : public TypeInfo {
    std::unique_ptr<TypeInfo> baseType;
    bool isArray() { return true; }
    TypeInfo *memberType(const std::string &ident) { return baseType.get(); }
};

// (i64, i64):i64 (regard "func" as define keyword like var)
struct FuncType : public TypeInfo {
    std::vector<std::unique_ptr<TypeInfo>> paramType;
    std::unique_ptr<TypeInfo> returnType;
    virtual bool isFunction() { return true; }
};

// {i64; i64} (access by '[num]' like tuple in python) | {x f32; y f32} (regard "type" as define keyword like var, TODO:
// "struct", "class" and "dynamic" as specific identifier like "addable") TODO: static, and regard member function as
// static func pointer
struct DynamicType : public TypeInfo {
    std::vector<std::tuple<std::string, std::unique_ptr<TypeInfo>>> memberType;
    TypeInfo *memberType(const std::string &ident) {
        return std::get<1>(*std::find_if(memberType.begin(), memberType.end(),
                                         [&](auto &x) { return std::get<0>(x) == ident; }))
            //*std::ranges::find_if(memberType, [&](auto &x) { return std::get<0>(x) == ident; }))
            .get();
    }
};

} // namespace rulejit