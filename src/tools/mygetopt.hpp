/**
 * @file mygetopt.hpp
 * @author djw
 * @brief 
 * @date 2023-04-26
 * 
 * @details tools to create command line tools
 * 
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-04-26</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

#include <format>
#include <iostream>
#include <map>
#include <ranges>
#include <set>
#include <string>
#include <string_view>
#include <tuple>

#include "stringprocess.hpp"
#include "myassert.hpp"

namespace tools::myopt {

struct CommandLineOpt {

    CommandLineOpt() : valueMap(), paramNum() {}

    /**
     * @brief register a argument(a string)
     * 
     * @param param name set of the parameter
     * @param info description of the parameter
     */
    void registerArg(const std::set<std::string> &param, const std::string &info = "") {
        paramNum[param] = std::make_tuple(true, info, paramNum.size());
    }

    /**
     * @brief register a flag(a bool)
     * 
     * @param param name set of the parameter
     * @param info description of the parameter
     */
    void registerFlag(const std::set<std::string> &param, const std::string &info = "") {
        paramNum[param] = std::make_tuple(false, info, paramNum.size());
    }

    /**
     * @brief return number of arguments, or -1 if error
     *
     * @param argc
     * @param argv
     * @return int
     */
    int build(int argc, const char **argv) {
        int cnt = 0;
        for (int i = 1; i < argc; ++i) {
            std::string s{argv[i]};
            auto it = std::ranges::find_if(paramNum, [&](const auto &p) { return p.first.contains(s); });
            cnt++;
            if (it == paramNum.end()) {
                unspecifiedValue.insert(s);
            } else {
                if (std::get<0>(it->second)) {
                    ++i;
                    if (i == argc) {
                        std::cout << "No value specified for " << s << std::endl;
                        return -1;
                    }
                    valueMap[std::get<2>(it->second)] = argv[i];
                }
                valueMap[std::get<2>(it->second)] = "";
            }
        }
        return cnt;
    }

    /**
     * @brief return number of arguments, or -1 if error
     *
     * @param args debug inputs
     */
    int debugBuild(std::vector<std::string> &args) {
        int cnt = 0;
        for (auto i = args.begin(); i != args.end(); ++i) {
            auto& s = *i;
            auto it = std::ranges::find_if(paramNum, [&](const auto &p) { return p.first.contains(s); });
            cnt++;
            if (it == paramNum.end()) {
                unspecifiedValue.insert(s);
            } else {
                if (std::get<0>(it->second)) {
                    ++i;
                    if (i == args.end()) {
                        std::cout << "No value specified for " << s << std::endl;
                        return -1;
                    }
                    valueMap[std::get<2>(it->second)] = *i;
                }
                valueMap[std::get<2>(it->second)] = "";
            }
        }
        return cnt;
    }

    /**
     * @brief get the input value of a flag
     * 
     * @param defaultValue default value
     * @param param name of the flag
     * @return bool value of the flag
     */
    bool getFlag(bool defaultValue, const std::string &param) {
        auto it = std::ranges::find_if(paramNum, [&](const auto &p) { return p.first.contains(param); });
        my_assert(it != paramNum.end());
        auto id = std::get<2>(it->second);
        my_assert(!std::get<0>(it->second));
        auto it2 = valueMap.find(id);
        if (it2 == valueMap.end()) {
            // no value
            return defaultValue;
        }
        return true;
    }

    /**
     * @brief get the input value of a argument
     * 
     * @param defaultValue default value
     * @param param name of the argument
     * @return const std::string& value of the argument
     */
    const std::string &getArg(const std::string &defaultValue, const std::string &param) {
        auto it = std::ranges::find_if(paramNum, [&](const auto &p) { return p.first.contains(param); });
        my_assert(it != paramNum.end());
        auto id = std::get<2>(it->second);
        my_assert(std::get<0>(it->second));
        auto it2 = valueMap.find(id);
        if (it2 == valueMap.end()) {
            // no value
            return defaultValue;
        }
        return it2->second;
    }

    /**
     * @brief assemble the help infomation to print
     *
     * @return std::string
     */
    std::string getHelp() {
        using namespace std::views;
        std::map<std::string, std::tuple<bool, std::string_view>> param;
        size_t maxIdent = 0;
        for (auto &[ns, v] : paramNum) {
            auto s = mystr::join(ns, " | ") + " ";
            maxIdent = std::max(maxIdent, s.size());
            param.emplace(std::move(s),
                          std::tuple<bool, std::string_view>(std::get<0>(v), std::string_view(std::get<1>(v))));
        }
        auto printLine = [maxIdent](auto &p) {
            return p.first + mystr::repeat(" ", maxIdent - p.first.size()) +
                   (std::get<0>(p.second) ? "<value>  " : "         ") + std::string(std::get<1>(p.second));
        };
        return head + std::format(
            "Options:\n\n{}\n\nFlags:\n\n{}\n",
            (param | filter([](auto &p) { return std::get<0>(p.second); }) | transform(printLine) | mystr::join("\n")),
            (param | filter([](auto &p) { return !std::get<0>(p.second); }) | transform(printLine) |
             mystr::join("\n")));
    }

    /**
     * @brief ask usr if continue
     * 
     * @param info 
     * @return true 
     * @return false 
     */
    bool askIfContinue(const std::string& info){
        std::cout << info << "[Y/n]" << std::endl;
        std::string input;
        std::cin >> input;
        if (input == "n" || input == "N") {
            return false;
        } else if (input != "" && input != "y" && input != "Y") {
            std::cout << "invalid input, abort." << std::endl;
            return false;
        }
        return true;
    }

    /**
     * @brief describe the usage of the program, will be the first line of help
     * 
     */
    std::string head;
    std::map<size_t, std::string> valueMap;
    std::set<std::string> unspecifiedValue;

  private:
    /// @brief {param names} -> (isArg, info, id)
    std::map<std::set<std::string>, std::tuple<bool, std::string, size_t>> paramNum;
};

} // namespace tools::myopt