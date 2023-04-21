/**
 * @file type.hpp
 * @author djw
 * @brief AST/Type
 * @date 2023-03-27
 *
 * @details Includes data structures hold and parse type info
 *
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-27</td><td>Initial version.</td></tr>
 * <tr><td>djw</td><td>2023-03-30</td><td>Change layout of TypeInfo.</td></tr>
 * </table>
 */
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
#include "tools/seterror.hpp"

namespace rulejit {

/**
 * @ingroup ast
 * @brief Structural type info
 *
 */
struct TypeInfo {
    friend struct TypeParser;

    TypeInfo() = default;
    TypeInfo(TypeInfo &&t) = default;
    TypeInfo(const TypeInfo &t) = default;
    TypeInfo(std::string &&s) : ident(std::move(s)), tokens(), subTypes() {}
    TypeInfo(const std::string &s) : ident(s), tokens(), subTypes() {}
    TypeInfo &operator=(const TypeInfo &t) = default;
    TypeInfo &operator=(TypeInfo &&t) = default;

    /**
     * @brief "spaceship operators" provides compare function between TypeInfo,
     * allows TypeInfo to act as key in std::set or std::map
     *
     */
    std::strong_ordering operator<=>(const TypeInfo &other) const {
        auto tmp = ident <=> other.ident;
        // my_assert(tmp != std::strong_ordering::equivalent);
        if (tmp == std::strong_ordering::equivalent) {
            if (subTypes.size() < other.subTypes.size()) {
                return std::strong_ordering::less;
            } else if (subTypes.size() > other.subTypes.size()) {
                return std::strong_ordering::greater;
            } else {
                for (size_t i = 0; i < subTypes.size(); ++i) {
                    auto tmp2 = subTypes[i] <=> other.subTypes[i];
                    if (tmp2 != std::strong_ordering::equivalent) {
                        return tmp2;
                    }
                }
                return std::strong_ordering::equivalent;
            }
        } else {
            return tmp;
        }
    }

    // type | where({"T", "int"}, ...);
    // friend std::map<std::string, TypeInfo> where(){}

    struct TemplateParam {
        std::map<std::string, TypeInfo> data;
    };

    /**
     * @brief tool function to create template parameter table.
     * used like 'type | where({"T", "int"}, ...);' or 'type | where(matched);'
     *
     * @tparam Args
     * @param args
     * @return TemplateParam
     */
    template <typename... Args> static TemplateParam where(Args... args) {
        return {std::map<std::string, TypeInfo>{std::forward<Args>(args)...}};
    }

    /**
     * @brief operator| used in template specialization
     *
     * @param table template param table
     * @return TypeInfo specialized type
     */
    [[nodiscard]] TypeInfo operator|(const TemplateParam &table) const {
        if (isBaseType()) {
            if (auto it = table.data.find(ident); it != table.data.end()) {
                return it->second;
            }
        }
        TypeInfo tmp = *this;
        for (auto &&sub : tmp.subTypes) {
            sub = sub | table;
        }
        return tmp;
    }

    /**
     * @brief template match
     *
     * @attention user need to check if matched contains all std::string in templateParam
     * (through templateParam.size() == matched.size()) if return true to avoid partly matched;
     *
     * @param tar TypeInfo need match
     * @param templateParam template args
     * @param[out] matched empty map to receice match result.
     *
     * @return bool, true if matched
     */
    bool match(const TypeInfo &tar, const std::set<std::string> &templateParam,
               std::map<std::string, TypeInfo> &matched) const {
        // ATTENTION: complex type match are not allowed
        my_assert(ident != "struct" && ident != "class" && ident != "dynamic" && tokens.size() == 0);
        if (isBaseType()) {
            if (auto it = templateParam.find(ident); it != templateParam.end()) {
                // this is a template param
                if (auto it2 = matched.find(ident); it2 != matched.end()) {
                    // template param already matched, check if constistent
                    if (it2->second != tar) {
                        return false;
                    }
                } else {
                    matched.emplace(*it, tar);
                }
                return true;
            }
        }
        // this is not template param, try to completely match tar.
        if (ident != tar.ident || subTypes.size() != tar.subTypes.size()) {
            return false;
        }
        // match subTypes
        for (auto ind : std::views::iota(size_t(0), subTypes.size())) {
            auto &sub = subTypes[ind];
            auto &tarSub = tar.subTypes[ind];
            if (!sub.match(tarSub, templateParam, matched)) {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief template match without templateParam constraint
     *
     * @attention user need to check if matched contains Type already defined
     *
     * @param tar TypeInfo need match
     * @param[out] matched empty map to receice match result.
     *
     * @return bool, true if matched
     */
    [[deprecated]] bool freeMatch(const TypeInfo &tar, std::map<std::string, TypeInfo> &matched) const {
        // ATTENTION: complex type match are not allowed
        my_assert(ident != "struct" && ident != "class" && ident != "dynamic" && tokens.size() == 0);
        if (isBaseType()) {
            if (!tar.isBaseType() || ident != tar.ident) {
                // regards every base type a template param
                if (auto it2 = matched.find(ident); it2 != matched.end()) {
                    // template param already matched, check if constistent
                    if (it2->second != tar) {
                        return false;
                    }
                } else {
                    matched.emplace(ident, tar);
                }
                return true;
            }
        }
        // this is not template param, try to completely match tar.
        if (ident != tar.ident || subTypes.size() != tar.subTypes.size()) {
            return false;
        }
        // match subTypes
        for (auto ind : std::views::iota(size_t(0), subTypes.size())) {
            auto &sub = subTypes[ind];
            auto &tarSub = tar.subTypes[ind];
            if (!sub.freeMatch(tarSub, matched)) {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief default comparision operator
     *
     * @return bool
     */
    bool operator==(const TypeInfo &) const = default;

    /**
     * @brief check if this is a valid type
     *
     * @return bool
     */
    bool isValid() const { return ident != typeident::NoInstanceTypeIdent; }

    /**
     * @brief get human-readable form string of this type.
     * permitted to be able to reconstruct from generated string through TypeParser
     *
     * @see TypeParser
     *
     * @return std::string type string
     */
    std::string toString() const {
        std::string res;
        if (ident[0] == '[' || ident[0] == '*' || ident == "const") {
            res += ident;
            if (ident == "const") {
                res += " ";
            }
            return res + subTypes[0].toString();
        }
        if (ident == "func") {
            res = "func(";
            bool start = true;
            // for func type, subTypes contains at least one element, so no need to check it == subTypes.end()
            for (auto it = subTypes.begin(); it != subTypes.end() - 1; ++it) {
                if (start) {
                    start = false;
                } else {
                    res += ", ";
                }
                res += it->toString();
            }
            res += ")";
            if (subTypes.back().isValid()) {
                // returned function; if not returned, do nothing
                res += ":" + subTypes.back().toString();
            }
        } else if (ident == "struct" || ident == "class" || ident == "dynamic") {
            res += ident;
            res += "{";
            auto it1 = tokens.begin();
            auto it2 = subTypes.begin();
            for (; it1 != tokens.end(); ++it1, ++it2) {
                res += *it1 + " " + it2->toString() + ";";
            }
            res += "}";
        } else if (isValid()) {
            my_assert(isBaseType());
            return ident;
        } else {
            // debug only, cannot recognize by TypeParser
            return "[[void]]";
        }
        return res;
    }

    /**
     * @brief check if this type is base type, which means it is not
     * function/complex/pointer/array or const type,
     * mainly a build-in type or user defined type.
     *
     * @return bool
     */
    bool isBaseType() const {
        return isValid() && ident != "func" && ident != "struct" && ident != "class" && ident != "dynamic" &&
               ident != "*" && ident != "const" && ident[0] != '*' && subTypes.empty();
    }

    /**
     * @brief get base type as string
     *
     * @attention only basetype(whose TypeInfo::isBaseType() returns true) can call this function.
     *
     * @return std::string base type string
     */
    std::string getBaseTypeString() const {
        my_assert(isBaseType(), "only base type can call getBaseTypeString()");
        return ident;
    }

    /**
     * @brief get type without undecoration of pointer, array or const.
     *
     * @return TypeInfo
     */
    TypeInfo getUndecorateType() const {
        TypeInfo tmp = *this;
        while (!tmp.isUndecorateType()) {
            tmp = std::move(tmp.subTypes[0]);
        }
        return tmp;
    }

    /**
     * @brief return if this type is without decoration of pointer, array or const.
     *
     * @return bool
     */
    bool isUndecorateType() const { return !(ident == "*" || ident == "const" || ident[0] == '['); }

    /**
     * @brief return if this type a complex type
     *
     * @return bool
     */
    bool isComplexType() const { return isValid() && (ident == "struct" || ident == "class" || ident == "dynamic"); }

    /**
     * @brief check if complex type have given member
     *
     * @param token member name need to be checked
     * @return bool
     */
    bool hasMember(std::string token) const {
        my_assert(isComplexType(), "only complex type has member");
        auto it = std::find(tokens.begin(), tokens.end(), token);
        return it != tokens.end();
    }

    /**
     * @brief get type of member with given name
     *
     * @param token member name
     * @return const TypeInfo& type of that member
     */
    const TypeInfo &getMemberType(std::string token) const {
        my_assert(isComplexType(), "only complex type has member");
        auto it = std::find(tokens.begin(), tokens.end(), token);
        my_assert(it != tokens.end(), "member not found: " + token);
        return subTypes[it - tokens.begin()];
    }

    /**
     * @brief get name of the index-th one member
     *
     * @param index member index
     * @return const std::string& member name
     */
    const std::string &getMemberName(size_t index) const {
        my_assert(isComplexType(), "only complex type has member");
        return tokens[index];
    }

    /**
     * @brief get count of member this complex type have
     *
     * @return size_t count of member
     */
    size_t getMemberCount() const {
        my_assert(isComplexType(), "only complex type has member");
        my_assert(tokens.size() == subTypes.size(), "idents and subTypes size mismatch");
        return subTypes.size();
    }

    /**
     * @brief check if this type is function type
     *
     * @return bool
     */
    bool isFunctionType() const { return isValid() && ident == "func"; }

    /**
     * @brief check if this function type has no return value
     *
     * @attention (isNoReturnFunctionType() xor isReturnedFunctionType()) maybe not true
     *
     * @see TypeInfo::isReturnedFunctionType
     *
     * @return bool
     */
    bool isNoReturnFunctionType() const { return isFunctionType() && !subTypes.back().isValid(); }

    /**
     * @brief check if this function type has return value
     *
     * @attention (isNoReturnFunctionType() xor isReturnedFunctionType()) maybe not true
     *
     * @see TypeInfo::isNoReturnFunctionType
     *
     * @return bool
     */
    bool isReturnedFunctionType() const { return isFunctionType() && subTypes.back().isValid(); }

    /**
     * @brief get the type of index-th argument of this function type
     *
     * @param index index of argument
     * @return const TypeInfo& type of that arg
     */
    const TypeInfo &getParamType(size_t index) const {
        my_assert(isFunctionType(), "only function type has arg");
        my_assert(index < subTypes.size(), "index out of range");
        return subTypes[index];
    }

    /**
     * @brief get count of arguments this function type need
     *
     * @return size_t count of arguments
     */
    size_t getParamCount() const {
        my_assert(isFunctionType(), "only function type has arg");
        return subTypes.size() - 1;
    }

    /**
     * @brief get returned type of this function type
     * @attention no need to check isReturnedFunctionType(), will return NoInstanceType if no return
     *
     * @return const TypeInfo& returned type
     */
    const TypeInfo &getReturnedType() const {
        my_assert(isFunctionType(), "only function type has return type");
        return subTypes.back();
    };

    /**
     * @brief function used to build function type directly,
     *
     * TODO: need change
     *
     */
    void addParamType(const TypeInfo &type) {
        my_assert(isFunctionType());
        subTypes.push_back(type);
    }

    /**
     * @brief function used to build function type directly,
     *
     * TODO: need change
     *
     */
    void addParamType(TypeInfo &&type) {
        my_assert(isFunctionType());
        subTypes.push_back(std::move(type));
    }

    /**
     * @brief check if this type is pointer type
     *
     * @return bool
     */
    bool isPointerType() const { return isValid() && ident == "*"; }

    /**
     * @brief get type that is pointer to this type
     *
     * @return TypeInfo
     */
    TypeInfo getPointerType() const {
        TypeInfo res;
        res.ident = "*";
        res.subTypes.push_back(*this);
        return res;
    }

    /**
     * @brief check if this type is array type
     *
     * @return bool
     */
    bool isArrayType() const { return isValid() && ident[0] == '['; }

    /**
     * @brief get element type of this array type
     *
     * @return TypeInfo
     */
    TypeInfo getElementType() const {
        my_assert(isArrayType(), "only array type has element");
        return subTypes[0];
    }

    /**
     * @brief get the ident
     *
     * @return const std::string&
     */
    const std::string &getIdent() const { return ident; }

    /**
     * @brief get the tokens object
     *
     * @return const std::vector<std::string>&
     */
    const std::vector<std::string> &getTokens() const { return tokens; }

    /**
     * @brief get the subTypes
     *
     * @return const std::vector<TypeInfo>&
     */
    const std::vector<TypeInfo> &getSubTypes() const { return subTypes; }

  private:
    std::string ident;
    std::vector<std::string> tokens;
    std::vector<TypeInfo> subTypes;
};

/**
 * @brief Parser which can get TypeInfo from string.
 * @see TypeInfo
 *
 */
struct TypeParser {

    /**
     * @brief pipe operator| to parse a TypeInfo from ExpressionLexer stream
     *
     * @param e ExpressionLexer which lexer the input string
     * @param t TypeParser to be exactly called parse function, normally a temporary object
     * @return TypeInfo
     */
    friend TypeInfo operator|(ExpressionLexer &e, const TypeParser &t) { return t.parse(e); }

  private:
    SET_ERROR_MEMBER("Type Parse", TypeInfo)
    // ExpressionLexer::Guidence end;
    static TypeInfo parse(ExpressionLexer &e) {
        TypeInfo info;
        constexpr auto ignore_break = ExpressionLexer::Guidence::IGNORE_BREAK;
        if (e.top() == "[" || e.top() == "*" || e.top() == "const") {
            if (e.top() == "[") {
                // '[' ']' *type
                // slice type: {"[]", *type}
                // TODO: '[' (num | ...) ']' *type
                // TODO: array type: {"[num]", *type}
                info.ident = "[]";
                e.pop(ignore_break);
                if (e.pop(ignore_break) != "]") {
                    return setError("mismatch \"[\" in slice type");
                }
            } else {
                info.ident = e.topCopy();
                e.pop(ignore_break);
            }
            info.subTypes.push_back(parse(e));
            return info;
        }
        if (e.tokenType() == TokenType::IDENT) {
            // TODO: template
            //     hard, when a & b is var, a<b>(0) is expression;
            //           when a & b is type, it is template function call.
            //     introduce context to parser?
            //     no, use <b>a instead. no contradiction now(no unary operator <)
            info.ident = e.popCopy();
            return info;
        } else if (e.top() == "func") {
            // type of member function with define "func foo (recv Recv)()->{}" is "func(Recv)", only log to "member
            // function table"
            // 'func' ('[' (VARDEF | IDENT)* ']')? '(' (type (',' type)*)? ')' (':' type)? func type:
            // {"func", "(", (type, (",", type,)*)? ")", ":", (type | "")}
            // TODO: closure: {"closure", "[", (type, ",",)* "]", "(", (type, (",", type,)*)? ")", (type | "")}
            info.ident = e.popCopy(ignore_break);
            if (e.pop(ignore_break) != "(") {
                return setError("expect \"(\", found: " + e.topCopy());
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
                    return setError("expect \"(\" in func type, found: " + e.topCopy());
                }
            }
            e.pop();
            if (e.top() != ":") {
                info.subTypes.push_back(TypeInfo(std::string(typeident::NoInstanceTypeIdent)));
                return info;
            } else {
                e.pop(ignore_break);
            }
            auto returnType = e | TypeParser();
            info.subTypes.push_back(returnType);
            return info;
        } else if (e.top() == "struct" || e.top() == "dynamic" || e.top() == "class") {
            // "struct"|"class"|"dynamic", "{", (ident, type, ";",)* "}"
            // complex def: {"struct"|"class"|"dynamic", "{", (ident, type, ";",)* "}"}
            info.ident = e.popCopy(ignore_break);
            if (e.pop(ignore_break) != "{") {
                return setError("expect \"{\", found: " + e.topCopy());
            }
            while (e.top() != "}") {
                if (e.tokenType() != TokenType::IDENT) {
                    return setError("expect ident, found: " + e.topCopy());
                }
                info.tokens.push_back(e.popCopy(ignore_break));
                auto memberType = e | TypeParser();
                info.subTypes.push_back(memberType);
                if (e.tokenType() != TokenType::ENDLINE && e.top() != "}") {
                    return setError("expect ENDLINE or \"}\", found: " + e.topCopy());
                }
                if (e.tokenType() == TokenType::ENDLINE) {
                    e.pop(ignore_break);
                }
            }
            e.pop();
            return info;
        } else {
            return setError("expect type identifier, found: " + e.topCopy());
        }
    };
};

/**
 * @brief tool function to make a TypeInfo from string without constructing ExpressionLexer
 *
 * @param type type string
 * @return TypeInfo
 */
inline TypeInfo make_type(const std::string &type) {
    static ExpressionLexer lexer;
    return type | lexer | TypeParser();
}

} // namespace rulejit