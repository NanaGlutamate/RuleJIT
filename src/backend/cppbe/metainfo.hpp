#pragma once

#include <any>
#include <set>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace rulejit::cppgen {

inline static std::set<std::string> baseData{
    "bool",  "int8",   "uint8",   "int16",   "uint16",   "int32",  "uint32",
    "int64", "uint64", "float32", "float64", "float128", "string",
};

struct MetaInfo {
    using CSValueMap = std::unordered_map<std::string, std::any>;
    MetaInfo() = default;
    MetaInfo(const MetaInfo &) = delete;
    std::vector<std::string> inputVar, outputVar, cacheVar;
    // name -> type
    std::unordered_map<std::string, std::string> varType;
    // type -> member name -> member type
    std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string>>> typeDefines;
};

} // namespace rulejit