#pragma once

#include <map>
#include <set>
#include <string>
#include <tuple>

namespace rulejit {

namespace typeident {

inline constexpr std::string_view StringType{"string"};
inline constexpr std::string_view RealType{"f64"};
inline constexpr std::string_view IntType{"f64"};

} // namespace typeident

inline const std::set<std::string_view> buildInType{
    typeident::StringType,
    typeident::IntType,
    typeident::RealType,
};

inline const std::set<std::string> reloadableBuildInUnary{
    "-", "not", "!", "&", "*", "~", "||", "&&",
};

using Priority = size_t;

inline const std::set<std::string_view> buildInMultiCharSymbol{
    "==", "!=", ">=", "<=", "&&", "||", "->", "...", ">>", "<<",
};

inline const std::map<std::string, Priority> reloadableBuildInInfix{
    {"*", 100}, {"/", 100},  {"+", 90},  {"-", 90},  {"<<", 80}, {">>", 80}, {">", 70},
    {"<", 70},  {">=", 70},  {"<=", 70}, {"==", 60}, {"!=", 60}, {"&", 50},  {"^", 40},
    {"|", 30},  {"and", 20}, {"&&", 20}, {"||", 10}, {"or", 10},
};

inline const std::set<std::string_view> defKeyWords{
    "func",
    "var",
    "type",
};

inline const std::set<std::string_view> keyWords{
    "if",      "else",   "while",  "func", "var", "type", "struct", "class",
    "dynamic", "extern", "return", "and",  "or",  "not",  "xor",
};

inline const std::set<std::string_view> typeIndicator{
    "struct", "class", "dynamic", "func", "[", "closure",
};

inline const std::set<std::string_view> lambdaKeyWords{
    "func",
};

} // namespace rulejit