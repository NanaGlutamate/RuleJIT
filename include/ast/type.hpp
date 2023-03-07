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

// all unnamed complex type is NOT equal to each other
struct TypeInfo {
    std::vector<std::string> idents;
    // bool isFunction(){
    //     return idents.size() > 1 && idents[0] == "func";
    //     // TODO: named function
    // }
    bool isValid() { return idents.size() >= 1 && idents[0] != ""; }
    std::string toString() {
        // TODO:
        if (idents.empty()) {
            return "[NO_TYPE]";
        }
        std::string res;
        for (auto &ident : idents) {
            res += ident;
        }
        return res;
    }
    bool isDefinedType() { return idents.size() == 1 && !buildInType.contains(idents[0]); }
    bool isComplexType() {
        return idents.size() > 1 && (idents[0] == "struct" || idents[0] == "class" || idents[0] == "dynamic");
    }
    // {"struct"|"class"|"dynamic", "{", (ident, type, ";",)* "}"}
    TypeInfo memberType(std::string token);
};

inline const TypeInfo voidType{};

struct TypeParser {
    // may leave '\n' as a ENDLINE
    // that means, last 'pop' called by TypeParser is with no Guidence
    friend TypeInfo operator|(ExpressionLexer &e, const TypeParser &t) { return t.parse(e); }

  private:
    // ExpressionLexer::Guidence end;
    static TypeInfo parse(ExpressionLexer &e) {
        TypeInfo info;
        constexpr auto ignore_break = ExpressionLexer::Guidence::IGNORE_BREAK;
        while(e.top() == "["){
            // '[' ']' *type
            // slice type: {"[]", *type}
            // TODO: '[' (num | ...) ']' *type
            // TODO: array type: {"[num]", *type}
            info.idents.push_back("[]");
            e.pop(ignore_break);
            if (e.pop(ignore_break) != "]") {
                return {};
            }
        }
        if (e.top() == "func") {
            // type of member function with define "func (recv Recv) foo ()->{}" is "func(Recv)", only log to "member function table"
            // 'func' ('[' (VARDEF | IDENT)* ']')? '(' (type (',' type)*)? ')' (':' type)?
            // func type: {"func", "(", (type, (",", type,)*)? ")", ":", (type | "")}
            // TODO: closure: {"closure", "[", (type, ",",)* "]", "(", (type, (",", type,)*)? ")", (type | "")}
            info.idents.push_back(e.popCopy(ignore_break));
            if (e.pop(ignore_break) != "(") {
                return {};
            }
            info.idents.push_back("(");
            while (e.top() != ")") {
                auto paramType = e | TypeParser();
                if (!paramType.isValid()) {
                    return {};
                }
                info.idents.push_back(paramType.toString());
                if (e.top() == ",") {
                    info.idents.push_back(",");
                    e.pop(ignore_break);
                } else if (e.top() != ")") {
                    return {};
                }
            }
            e.pop();
            info.idents.push_back(")");
            info.idents.push_back(":");
            if (e.top() != ":") {
                info.idents.push_back("");
                return info;
            }
            if(e.top() == "\n"){
                e.pop(ignore_break);
            }
            auto returnType = e | TypeParser();
            if (!returnType.isValid()) {
                return {};
            }
            info.idents.push_back(returnType.toString());
            return info;
        } else if (e.top() == "struct" || e.top() == "dynamic" || e.top() == "class") {
            // "struct"|"class"|"dynamic", "{", (ident, type, ";",)* "}"
            // complex def: {"struct"|"class"|"dynamic", "{", (ident, type, ";",)* "}"}
            info.idents.push_back(e.popCopy(ignore_break));
            if (e.pop(ignore_break) != "{") {
                return {};
            }
            info.idents.push_back("{");
            while (e.top() != "}") {
                if (e.tokenType() != TokenType::IDENT) {
                    return {};
                }
                // TODO: unnamed member?
                info.idents.push_back(e.popCopy(ignore_break));
                auto memberType = e | TypeParser();
                if (!memberType.isValid()) {
                    return {};
                }
                info.idents.push_back(memberType.toString());
                if(e.tokenType() != TokenType::ENDLINE){
                    return {};
                }
                e.pop(ignore_break);
                info.idents.push_back(";");
            }
            e.pop();
            return info;
        } else if (e.tokenType() == TokenType::IDENT) {
            info.idents.push_back(e.popCopy());
            return info;
        } else {
            return {};
        }
    };
};

inline TypeInfo make_type(const std::string& type) {
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