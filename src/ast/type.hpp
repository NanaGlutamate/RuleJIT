#pragma once

#include <algorithm>
#include <map>
#include <memory>
#include <ranges>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include "defines/language.hpp"
#include "frontend/lexer.h"
#include "tools/myassert.hpp"

namespace rulejit {

// struct TypeInfo{
//     virtual ~TypeInfo()=default;
// };

// struct TypeTokenList : public TypeToken{
//     std::vector<std::unique_ptr<TypeInfo>> tokens;
//     template <typename V>
//     TypeTokenList(V&& v) : idents(std::forward<V>(v)) {}
// };

// struct TypeToken : public TypeInfo/*, public std::string*/{
//     std::string token;
//     template<typename S>
//     TypeToken(S&& s) : token(std::forward<S>(s)) {}
// };

// all unnamed complex type is NOT equal to each other
struct TypeInfo {
    std::vector<std::string> idents;
    TypeInfo() = default;
    template <typename S>
    TypeInfo(S&& s):idents(std::forward<S>(s)){};
    TypeInfo(TypeInfo&& t) = default;
    TypeInfo(const TypeInfo& t) = default;
    // bool isFunction(){
    //     return idents.size() > 1 && idents[0] == "func";
    //     // TODO: named function
    // }
    operator bool() const { return isValid(); }
    // TODO: unnamed complex type or array/slice of unnamed complex type not equal
    bool operator==(const TypeInfo &other) const {
        if (isComplexType() || other.isComplexType()) {
            return false;
        }
        if (idents.size() != other.idents.size()) {
            return false;
        }
        auto it1 = idents.begin();
        auto it2 = other.idents.begin();
        while (it1 != idents.end() && it2 != other.idents.end()) {
            if (*it1 != *it2) {
                return false;
            }
            ++it1;
            ++it2;
        }
    }
    bool isValid() const { return idents.size() >= 1 && idents[0] != ""; }
    std::string baseType() const {
        std::string res;
        auto it = idents.begin();
        while (it != idents.end() && (*it == "[]" || *it == "*")) {
            ++it;
        }
        while (it != idents.end()) {
            res += *it;
            ++it;
        }
        return res;
    }
    std::string toString() const {
        // TODO:
        if (!isValid()) {
            return "";
        }
        std::string res;
        for (auto &ident : idents) {
            res += ident + " ";
        }
        return res;
    }
    bool isSingleToken() const { return idents.size() == 1; }
    bool isDefinedType() const { return isSingleToken() && !buildInType.contains(idents[0]); }
    bool isComplexType() const {
        return idents.size() > 1 && (idents[0] == "struct" || idents[0] == "class" || idents[0] == "dynamic");
    }
    // {"struct"|"class"|"dynamic", "{", (ident, type, ";",)* "}"}
    TypeInfo memberType(std::string token);
};

inline const TypeInfo NoInstanceType{std::vector<std::string>{std::string(typeident::NoInstanceTypeIdent)}};
inline const TypeInfo StringType{std::vector<std::string>{std::string(typeident::StringTypeIdent)}};
inline const TypeInfo IntType{std::vector<std::string>{std::string(typeident::IntTypeIdent)}};
inline const TypeInfo RealType{std::vector<std::string>{std::string(typeident::RealTypeIdent)}};

struct TypeParser {
    // may leave '\n' as a ENDLINE
    // that means, last 'pop' called by TypeParser is with no Guidence
    friend TypeInfo operator|(ExpressionLexer &e, const TypeParser &t) { return t.parse(e); }

  private:
    [[noreturn]] static TypeInfo error(const std::string &err) { throw std::logic_error("Type Parse Error: " + err); }
    // ExpressionLexer::Guidence end;
    static TypeInfo parse(ExpressionLexer &e) {
        TypeInfo info;
        constexpr auto ignore_break = ExpressionLexer::Guidence::IGNORE_BREAK;
        while (e.top() == "[" || e.top() == "*") {
            if (e.top() == "*") {
                info.idents.push_back("*");
                e.pop(ignore_break);
            } else {
                // '[' ']' *type
                // slice type: {"[]", *type}
                // TODO: '[' (num | ...) ']' *type
                // TODO: array type: {"[num]", *type}
                info.idents.push_back("[]");
                e.pop(ignore_break);
                if (e.pop(ignore_break) != "]") {
                    return error("mismatch \"[\" in slice type");
                }
            }
        }
        if (e.top() == "func") {
            // type of member function with define "func (recv Recv) foo ()->{}" is "func(Recv)", only log to "member
            // function table" 'func' ('[' (VARDEF | IDENT)* ']')? '(' (type (',' type)*)? ')' (':' type)? func type:
            // {"func", "(", (type, (",", type,)*)? ")", ":", (type | "")}
            // TODO: closure: {"closure", "[", (type, ",",)* "]", "(", (type, (",", type,)*)? ")", (type | "")}
            info.idents.push_back(e.popCopy(ignore_break));
            if (e.pop(ignore_break) != "(") {
                return error("expect \"(\", found: " + e.topCopy());
            }
            info.idents.push_back("(");
            while (e.top() != ")") {
                auto paramType = e | TypeParser();
                if(e.top()=="\n"){
                    e.pop(ignore_break);
                }
                info.idents.push_back(paramType.toString());
                if (e.top() == ",") {
                    info.idents.push_back(",");
                    e.pop(ignore_break);
                } else if (e.top() != ")") {
                    return error("mismatch \"(\" in func type");
                }
            }
            e.pop();
            info.idents.push_back(")");
            info.idents.push_back(":");
            if (e.top() != ":") {
                info.idents.push_back("");
                return info;
            } else {
                e.pop(ignore_break);
            }
            if (e.top() == "\n") {
                e.pop(ignore_break);
            }
            auto returnType = e | TypeParser();
            info.idents.push_back(returnType.toString());
            return info;
        } else if (e.top() == "struct" || e.top() == "dynamic" || e.top() == "class") {
            // "struct"|"class"|"dynamic", "{", (ident, type, ";",)* "}"
            // complex def: {"struct"|"class"|"dynamic", "{", (ident, type, ";",)* "}"}
            if(!info.idents.empty()) {
                return error("list of or pointer to unnamed complex structure is not allowed");
            }
            info.idents.push_back(e.popCopy(ignore_break));
            if (e.pop(ignore_break) != "{") {
                return error("expect \"{\", found: " + e.topCopy());
            }
            info.idents.push_back("{");
            while (e.top() != "}") {
                if (e.tokenType() != TokenType::IDENT) {
                    return error("expect ident, found: " + e.topCopy());
                }
                // TODO: unnamed member?
                info.idents.push_back(e.popCopy(ignore_break));
                auto memberType = e | TypeParser();
                info.idents.push_back(memberType.toString());
                if (e.tokenType() != TokenType::ENDLINE && e.top() != "}") {
                    return error("expect ENDLINE or \"}\", found: " + e.topCopy());
                }
                if (e.tokenType() == TokenType::ENDLINE) {
                    e.pop(ignore_break);
                }
                info.idents.push_back(";");
            }
            info.idents.push_back(e.popCopy());
            return info;
        } else if (e.tokenType() == TokenType::IDENT) {
            info.idents.push_back(e.popCopy());
            return info;
        } else {
            return error("expect type identifier, found: " + e.topCopy());
            ;
        }
    };
};

inline TypeInfo make_type(const std::string &type) {
    static ExpressionLexer lexer;
    return type | lexer | TypeParser();
}

inline TypeInfo TypeInfo::memberType(std::string token) {
    static ExpressionLexer lexer;
    if (!isComplexType()) {
        return {};
    }
    for (size_t start = 2; start < idents.size(); start += 3) {
        if (idents[start] == token) {
            return idents[start + 1] | lexer | TypeParser();
        }
    }
}

// // TYPE := IDENT | ARRAYTYPE | SLICETYPE | FUNCTYPE | COMPLEXTYPE
// struct TypeInfo {
//     virtual bool isArray() { return false; }
//     virtual bool isFunction() { return false; }
//     virtual bool isIdent() { return false; }
//     virtual TypeInfo *getMemberType(const std::string &ident) { return nullptr; }
//     // virtual bool isLegal(){return false;}
//     virtual std::unique_ptr<TypeInfo> copy() = 0;
//     virtual ~TypeInfo() = default;
// };

// // IDENT
// // a
// struct TypeIdent : public TypeInfo {
//     std::string name;
//     std::unique_ptr<TypeInfo> copy() override {
//         return std::make_unique<TypeIdent>(name);
//     }
//     virtual bool isIdent() { return true; }
// };

// // // [4]a | TODO: [5,5]a
// // struct ArrayType : public TypeInfo{
// //     size_t len;
// //     std::unique_ptr<TypeInfo> baseType;
// //     bool isArray(){return true;}
// // };

// // SLICETYPE := '[' ']' TYPE
// // []a
// struct SliceType : public TypeInfo {
//     std::unique_ptr<TypeInfo> baseType;
//     bool isArray() { return true; }
//     TypeInfo *getMemberType(const std::string &ident) { return baseType.get(); }
//     std::unique_ptr<TypeInfo> copy() override {
//         return std::make_unique<SliceType>(baseType->copy());
//     }
// };

// // FUNCTYPE := '(' (TYPE (',' TYPE)*)? ')' ('->' TYPE)?
// // (i64, i64)->i64 (regard "func" as define keyword like var)
// struct FuncType : public TypeInfo {
//     std::vector<std::unique_ptr<TypeInfo>> paramType;
//     // nullptr if no returns
//     std::unique_ptr<TypeInfo> returnType;
//     std::unique_ptr<TypeInfo> copy() override {
//         std::vector<std::unique_ptr<TypeInfo>> tmp;
//         for(auto& p : paramType){
//             tmp.push_back(p->copy());
//         }
//         return std::make_unique<FuncType>(
//             std::move(tmp),
//             returnType->copy()
//         );
//     }
//     virtual bool isFunction() { return true; }
// };

// // COMPLEXTYPE := ('struct' | 'class' | 'dynamic') '{' (TYPE (ENDLINE TYPE)*)? '}'
// // {i64; i64} (access by '[num]' like tuple in python) | {x f32; y f32} (regard "type" as define keyword like var,
// TODO:
// // "struct", "class" and "dynamic" as attribute like "addable") TODO: static, and regard member function as static
// func
// // pointer
// struct DynamicType : public TypeInfo {
//     std::vector<std::tuple<std::string, std::unique_ptr<TypeInfo>>> memberType;
//     TypeInfo *getMemberType(const std::string &ident) {
//         return std::get<1>(*std::find_if(memberType.begin(), memberType.end(),
//                                          [&](auto &x) { return std::get<0>(x) == ident; }))
//             //*std::ranges::find_if(memberType, [&](auto &x) { return std::get<0>(x) == ident; }))
//             .get();
//     }
//     std::unique_ptr<TypeInfo> copy() override {
//         std::vector<std::tuple<std::string, std::unique_ptr<TypeInfo>>> tmp;
//         for(auto& p : memberType){
//             tmp.push_back(std::make_tuple(std::get<0>(p), std::get<1>(p)->copy()));
//         }
//         return std::make_unique<DynamicType>(
//             std::move(tmp)
//         );
//     }
// };

} // namespace rulejit