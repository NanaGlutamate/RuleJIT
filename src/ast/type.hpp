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

struct TypeInfo {
    // TODO: remove direct access to idents
    std::vector<std::string> idents; // ("[]" | "*")* (ident | "func" ":"? | struct (ident)* | class (ident)*)
    std::vector<TypeInfo> subTypes;  // func param type / complex member type
    TypeInfo() = default;
    TypeInfo(TypeInfo &&t) : idents(std::move(t.idents)), subTypes(std::move(t.subTypes)) {}
    TypeInfo(const TypeInfo &t) : idents(t.idents), subTypes(t.subTypes) {}
    TypeInfo &operator=(const TypeInfo &t) {
        idents = t.idents;
        subTypes = t.subTypes;
        return *this;
    }
    TypeInfo &operator=(TypeInfo &&t) {
        idents = std::move(t.idents);
        subTypes = std::move(t.subTypes);
        return *this;
    }

    TypeInfo(std::vector<std::string> &&s) : idents(std::move(s)), subTypes() {}
    TypeInfo(const std::vector<std::string> &s) : idents(s), subTypes() {}
    operator bool() const { return isValid(); }
    bool operator==(const TypeInfo &other) const {
        // if (isComplexType() || other.isComplexType()) {
        //     return false;
        // }
        return idents == other.idents && subTypes == other.subTypes;
    }
    bool operator!=(const TypeInfo &other) const { return !(*this == other); }
    bool isValid() const { return idents.size() >= 1 && idents[0] != ""; }
    std::string baseType() const {
        if (!isValid()) {
            return "";
        }
        std::string res;
        auto it = idents.begin();
        while (it != idents.end() && ((*it)[0] == '[' || *it == "*")) {
            ++it;
        }
        while (it != idents.end()) {
            res += *it;
            ++it;
        }
        return res;
    }
    std::string toString() const {
        std::string res;
        size_t real = 0;
        while (idents.size() > real && idents[real].size() >= 1 && idents[real][0] == '[' || idents[real][0] == '*' ||
               idents[real] == "const") {
            res += idents[real];
            if (idents[real] == "const")
                res += " ";
            ++real;
        }
        if (idents[real] == "func") {
            res = "func(";
            my_assert(idents.size() == real + 1 || (idents.size() == real + 2 && idents[real + 1] == ":"));
            bool start = true;
            if (subTypes.size() > idents.size() + real - 1) {
                for (auto it = subTypes.begin(); it != subTypes.end() - idents.size() + real + 1; ++it) {
                    if (start) {
                        start = false;
                    } else {
                        res += ",";
                    }
                    res += it->toString();
                }
            }
            res += ")";
            if (idents.size() == 2 + real) {
                res += ":" + subTypes.back().toString();
            }
        } else if (idents[real] == "struct" || idents[real] == "class" || idents[real] == "dynamic") {
            res += idents[real];
            res += "{";
            my_assert(idents.size() == subTypes.size() + 1 + real);
            auto it1 = idents.begin() + 1 + real;
            auto it2 = subTypes.begin();
            for (; it1 != idents.end(); ++it1, ++it2) {
                res += *it1 + " " + it2->toString() + ";";
            }
            res += "}";
        } else {
            my_assert(subTypes.size() == 0 && idents.size() - real == 1, "only 1 ident in type name allowed");
            res = res + idents.back();
        }
        if(res.empty()) {
            return "[[void]]";
        }
        return res;
    }
    bool isFunctionType() const { return isValid() && idents[0] == "func"; }
    bool isNoReturnFunctionType() const { return isFunctionType() && idents.size() == 1; }
    bool isReturnedFunctionType() const { return isFunctionType() && idents.size() == 2 && idents[1] == ":"; }
    const TypeInfo &getMemberType(std::string token) const {
        my_assert(isComplexType(), "only complex type has member");
        auto it = std::find(idents.begin() + 1, idents.end(), token);
        my_assert(it != idents.end(), "member not found: "+token);
        return subTypes[it - idents.begin() - 1];
    }
    const TypeInfo &getReturnedType() const;
    // not complex nor function nor pointer nor array type
    bool isArrayType() const { return isValid() && idents.size() >= 1 && idents[0][0] == '['; }

    const TypeInfo &getArgType(size_t index) const {
        my_assert(isFunctionType(), "only function type has arg");
        my_assert(index < subTypes.size(), "index out of range");
        return subTypes[index];
    }
    TypeInfo getElementType() const {
        my_assert(isArrayType(), "only array type has element");
        TypeInfo res = *this;
        res.idents.erase(res.idents.begin());
        return res;
    }
    bool isBaseType() const {
        return isValid() && idents.size() == 1 && idents[0] != "func" && idents[0] != "struct" &&
               idents[0] != "class" && idents[0] != "dynamic";
    }
    bool isComplexType() const {
        return isValid() && idents.size() >= 1 &&
               (idents[0] == "struct" || idents[0] == "class" || idents[0] == "dynamic");
    }
    // {"struct"|"class"|"dynamic", "{", (ident, type, ";",)* "}"}
    TypeInfo memberType(std::string token);
};

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
        while (e.top() == "[" || e.top() == "*" || e.top() == "const") {
            if (e.top() == "[") {
                // '[' ']' *type
                // slice type: {"[]", *type}
                // TODO: '[' (num | ...) ']' *type
                // TODO: array type: {"[num]", *type}
                info.idents.push_back("[]");
                e.pop(ignore_break);
                if (e.pop(ignore_break) != "]") {
                    return error("mismatch \"[\" in slice type");
                }
            } else {
                info.idents.push_back(e.topCopy());
                e.pop(ignore_break);
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
            while (e.top() != ")") {
                auto paramType = e | TypeParser();
                if (e.top() == "\n") {
                    e.pop(ignore_break);
                }
                info.subTypes.push_back(paramType);
                if (e.top() == ",") {
                    e.pop(ignore_break);
                } else if (e.top() != ")") {
                    return error("mismatch \"(\" in func type");
                }
            }
            e.pop();
            if (e.top() != ":") {
                return info;
            } else {
                info.idents.push_back(":");
                e.pop(ignore_break);
            }
            if (e.top() == "\n") {
                e.pop(ignore_break);
            }
            auto returnType = e | TypeParser();
            info.subTypes.push_back(returnType);
            return info;
        } else if (e.top() == "struct" || e.top() == "dynamic" || e.top() == "class") {
            // "struct"|"class"|"dynamic", "{", (ident, type, ";",)* "}"
            // complex def: {"struct"|"class"|"dynamic", "{", (ident, type, ";",)* "}"}
            if (!info.idents.empty()) {
                return error("list of or pointer to unnamed complex structure is not allowed");
            }
            info.idents.push_back(e.popCopy(ignore_break));
            if (e.pop(ignore_break) != "{") {
                return error("expect \"{\", found: " + e.topCopy());
            }
            while (e.top() != "}") {
                if (e.tokenType() != TokenType::IDENT) {
                    return error("expect ident, found: " + e.topCopy());
                }
                info.idents.push_back(e.popCopy(ignore_break));
                auto memberType = e | TypeParser();
                info.subTypes.push_back(memberType);
                if (e.tokenType() != TokenType::ENDLINE && e.top() != "}") {
                    return error("expect ENDLINE or \"}\", found: " + e.topCopy());
                }
                if (e.tokenType() == TokenType::ENDLINE) {
                    e.pop(ignore_break);
                }
            }
            e.pop();
            return info;
        } else if (e.tokenType() == TokenType::IDENT) {
            info.idents.push_back(e.popCopy());
            return info;
        } else {
            return error("expect type identifier, found: " + e.topCopy());
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
    for (size_t start = 1; start < idents.size(); start++) {
        if (idents[start] == token) {
            return subTypes[start - 1];
        }
    }
}

} // namespace rulejit