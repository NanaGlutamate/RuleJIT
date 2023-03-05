#pragma once

#include <set>
#include <string>
#include <map>

namespace rulejit {

inline const std::set<std::string_view> preOp{
    "-",
    "not",
    "!",
};

inline const std::map<std::string, int> buildInInfix{
    {".", 100}, {"*", 95},  {"/", 95},  {"+", 90},   {"-", 90},  {">", 80},  {"<", 80},  {">=", 80},
    {"<=", 80}, {"==", 75}, {"!=", 75}, {"and", 55}, {"&&", 55}, {"||", 50}, {"or", 50},
};

inline std::set<std::string_view> keyWords{
    "if", "else", "while", "func", "var", "type", "struct", "class", "dynamic", "extern", "return",
};

inline std::set<std::string_view> defKeyWords{
    "func", "var", "type",
};

namespace typeident{

inline constexpr std::string_view StringType{"string"};
inline constexpr std::string_view ValueType{"f64"};

}

inline std::set<std::string> buildInType{
    // "i64",
    // "u64",
    // "char",
    typeident::StringType,
    typeident::ValueType,
};

}