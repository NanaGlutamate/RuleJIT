/**
 * @file printcsvaluemap.hpp
 * @author djw
 * @brief Tools/CSValueMap printer
 * @date 2023-03-28
 *
 * @details Includes a tool function to print CSValueMap.
 *
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-28</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

#include <any>
#include <format>
#include <iostream>
#include <ranges>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "tools/anyprocess.hpp"
#include "tools/stringprocess.hpp"

namespace rulejit {

std::string printCSValueMapToString(const std::unordered_map<std::string, std::any> &v);

inline std::string printAnyToString(const std::any &a) {
    struct UnknownTypeHandler{
        std::string operator()(const std::any &v){
            return "-unknown-inner-type-";
        }
    };
    return myany::visit<UnknownTypeHandler>(
        [](const auto &a) {
            if constexpr (std::is_same_v<std::vector<std::any>, std::remove_cvref_t<decltype(a)>>) {
                return std::format("[{}]", mystr::join(a | std::views::transform(printAnyToString), ", "));
            } else if constexpr (std::is_same_v<std::unordered_map<std::string, std::any>,
                                                std::remove_cvref_t<decltype(a)>>) {
                return printCSValueMapToString(a);
            } else if constexpr (std::is_same_v<std::string, std::remove_cvref_t<decltype(a)>>) {
                return std::format("\"{}\"", a);
            } else {
                return std::to_string(a);
            }
        },
        a);
}

inline std::string printCSValueMapToString(const std::unordered_map<std::string, std::any> &v) {
    using namespace std;
    return std::format("{{{}}}", mystr::join(v | std::views::transform([](const auto &p) {
                                                 return std::format("{{{} : {}}}", p.first, printAnyToString(p.second));
                                             }),
                                             ", "));
}

/**
 * @brief tool function to print CSValueMap to std::cout
 *
 * @param v CSValuMap need to be printed
 */
inline void printCSValueMap(const std::unordered_map<std::string, std::any> &v) {
    std::cout << printCSValueMapToString(v) << std::endl;
}

} // namespace rulejit