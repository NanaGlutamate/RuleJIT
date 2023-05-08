/**
 * @file rulesetparser.h
 * @author djw
 * @brief FrontEnd/Ruleset
 * @date 2023-03-29
 *
 * @details
 *
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-29</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "ast/context.hpp"

namespace rulejit::ruleset {

/// @brief Supported data types in ruleset XML
inline static std::set<std::string> baseData{
    "bool",  "int8",   "uint8",   "int16",   "uint16",   "int32",  "uint32",
    "int64", "uint64", "float32", "float64", "float128", "string",
};

/// @brief Supported numerical data types in ruleset XML
inline static std::set<std::string> baseNumericalData{
    "bool",  "int8",   "uint8",   "int16",   "uint16",   "int32",  "uint32",
    "int64", "uint64", "float32", "float64", "float128",
};

/// @brief rule set meta-informations, collected from <MetaInfo> node
struct RuleSetMetaInfo {
    RuleSetMetaInfo() = default;
    RuleSetMetaInfo(const RuleSetMetaInfo &) = delete;
    RuleSetMetaInfo(RuleSetMetaInfo &&) = delete;
    RuleSetMetaInfo &operator=(const RuleSetMetaInfo &) = delete;
    RuleSetMetaInfo &operator=(RuleSetMetaInfo &&) = delete;

    /// @brief Stored input/output/cache variable names
    std::vector<std::string> inputVar, outputVar, cacheVar;
    /// @brief Stored variable types, name -> type
    std::unordered_map<std::string, std::string> varType;
    /// @brief Stored type defines, type -> member name -> member type
    std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string>>> typeDefines;
    /// @brief Vars each atom rule changes, subruleset ID -> atom rule ID -> changed var names
    std::vector<std::vector<std::set<std::string>>> modifiedValue;
};

/// @brief rule set structure, collected from xml or whatever
struct RuleSetStructure {
    /// @brief store vars which has init value
    struct InitValue{
        std::string name;
        std::string_view value;
    };
    /// @brief store vars which is intermediate var
    struct IntermediateValue{
        std::string name;
        std::string_view value;
    };
    /// @brief store subruleset
    struct SubRuleSet{
        /// @brief store atom rules
        struct AtomRule{
            struct Consequence{
                std::string target;
                std::string operation;
                std::vector<std::string_view> args;
            };
            std::string_view condition;
            std::vector<Consequence> consequences;
        };
        std::vector<AtomRule> atomRules;
    };

    std::vector<char> source;
    std::vector<size_t> lineBreaks;

    /// @brief Stored input/output/cache variable names
    std::vector<std::string> inputVar, outputVar, cacheVar;
    /// @brief Stored variable types, name -> type
    std::unordered_map<std::string, std::string> varType;
    /// @brief Stored type defines, type -> member name, member type
    std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string>>> typeDefines;

    std::vector<InitValue> initValues;
    std::vector<IntermediateValue> intermediateValues;
    std::vector<SubRuleSet> subRuleSets;
};

/// @brief returned information of readSource()
struct RuleSetParseInfo {
    /// @brief real function name of predefines
    std::string preDefines;
    /// @brief real function name of preprocess
    std::vector<std::string> preprocess;
    /// @brief real function names of subrulesets
    std::vector<std::string> subRuleSets;
};

/// @brief static class which contains tool functions for parsing ruleset XML
struct RuleSetParser {
    /**
     * @brief collect informations in ruleset XML, and load functiondef and input/output/cache
     * variable name to context
     * @attention will add type defines in XML to ContextStack given
     *
     * @param srcXML source of ruleset XML
     * @param[out] context ContextStack which will be modified
     * @param[out] data meta information of ruleset parser to fill
     * @return RuleSetParseInfo
     */
    static RuleSetParseInfo readSource(const std::string &srcXML, ContextStack &context, RuleSetMetaInfo &data);
};

} // namespace rulejit::ruleset