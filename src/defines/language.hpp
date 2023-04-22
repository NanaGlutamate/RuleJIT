/**
 * @file language.hpp
 * @author djw
 * @brief Defines/Language
 * @date 2023-03-28
 *
 * @details Includes some language-related defines.
 *
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-28</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

#include <map>
#include <set>
#include <string>
#include <tuple>

namespace rulejit {

namespace typeident {

inline constexpr std::string_view NoInstanceTypeIdent{""};
inline constexpr std::string_view StringTypeIdent{"string"};
inline constexpr std::string_view RealTypeIdent{"f64"};
inline constexpr std::string_view IntTypeIdent{"f64"};
inline constexpr std::string_view AutoTypeIdent{"auto"};

} // namespace typeident

namespace config {

inline constexpr bool ignoreAllBreak = false;
inline constexpr bool allowUserDefinedInfix = true;
// can not change now
inline constexpr bool canCallBeforeDefine = true;
// can not change now
inline constexpr bool allowUserDefinedUnary = false;
// can not change now
inline constexpr bool allowTopLevelExpr = true;

static_assert(canCallBeforeDefine + allowUserDefinedInfix + allowUserDefinedUnary < 3);

} // namespace config

inline const std::set<std::string> BUILDIN_UNARY{
    "-", "not", "!", "&", "*", "~", "or", "and", "*", "&", "<-", "->",
};

using Priority = int;

// A "<-" B := (*A) = B TODO: A < -B? use <<-
inline const std::set<std::string_view> BUILD_IN_MULTICHAR_SYMBOL{
    "==", "!=", ">=", "<=", "&&", "||", "->", "..", ">>", "<<", ":=", "=>",
};

// inline const std::set<std::string_view> unaryOnlyMultiCharSymbol{
//     "<-",
// };

constexpr Priority USER_DEFINED_PRIORITY = 5;
constexpr Priority ASSIGN_PRIORITY = 1;
inline const std::map<std::string, Priority> BUILDIN_INFIX{
    {"*", 100},
    {"/", 100},
    {"%", 100},
    {"+", 90},
    {"-", 90},
    {"<<", 80},
    {">>", 80},
    {">", 70},
    {"<", 70},
    {">=", 70},
    {"<=", 70},
    {"==", 60},
    {"!=", 60},
    {"&", 50},
    {"^", 40},
    {"xor", 40},
    {"|", 30},
    {"and", 20},
    {"&&", 20},
    {"||", 10},
    {"or", 10},

    {"=", ASSIGN_PRIORITY},
    // {":=", ASSIGN_PRIORITY},
};

inline const std::set<std::string_view> DEF_KEYWORDS{
    "func",
    "var",
    "type",
    "const",
};

inline const std::set<std::string_view> COMMAND_KEYWORDS{
    "import",
    "export",
    "extern",
};

// words in KEYWORD is regarded as SYM, not IDENT
inline const std::set<std::string_view> KEYWORDS{
    "if",     "else",   "until", "func", "var",   "type", "struct",   "class",  "dynamic",
    "extern", "return", "and",   "or",   "not",   "xor",  "continue", "import", "export",
    "while",  "for",    "const", "auto", "match", "when", "is",       "as",     "fit",
};

inline const std::set<std::string_view> RESERVED_NOT_RELOADABLE_SYMBOL{
    "=",      ":=",     "(",     ")",   "[",     "]",      "{",     "}",       ",",      ";",      "if",
    "else",   "until",  "func",  "var", "type",  "struct", "class", "dynamic", "extern", "return", "continue",
    "import", "export", "while", "for", "const", "auto",   "match", "when",    "is",     "as",     "fit",
};

} // namespace rulejit