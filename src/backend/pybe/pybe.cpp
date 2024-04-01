/**
 * @file pybe.cpp
 * @author djw
 * @brief CQ/PYBE/Cpp Engine
 * @date 2024-03-11
 *
 * @details Includes main logic of py-backend.
 *
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-27</td><td>Initial version.</td></tr>
 * </table>
 */
#include <filesystem>
#include <iostream>
#include <ranges>

#include "defines/marco.hpp"
#include "pybe.h"
#include "rapidxml-1.13/rapidxml.hpp"
#include "tools/seterror.hpp"
#include "tools/stringprocess.hpp"

namespace {

using namespace rulejit;
using namespace rulejit::pybe;

constexpr std::string_view pyFileHeader = R"(import copy
import math

import torch

def abs(x):
    if isinstance(x, torch.Tensor):
        return torch.abs(x)
    return math.abs(x)

def acos(x):
    if isinstance(x, torch.Tensor):
        return torch.acos(x)
    return math.acos(x)

def asin(x):
    if isinstance(x, torch.Tensor):
        return torch.asin(x)
    return math.asin(x)

def atan(x):
    if isinstance(x, torch.Tensor):
        return torch.atan(x)
    return math.atan(x)

def atan2(y, x):
    if isinstance(x, torch.Tensor) or isinstance(y, torch.Tensor):
        return torch.atan2(y, x)
    return math.atan2(y, x)

def cos(x):
    if isinstance(x, torch.Tensor):
        return torch.cos(x)
    return math.cos(x)

def exp(x):
    if isinstance(x, torch.Tensor):
        return torch.exp(x)
    return math.exp(x)

def floor(x):
    if isinstance(x, torch.Tensor):
        return torch.floor(x)
    return math.floor(x)

def pow(x, n):
    if isinstance(x, torch.Tensor):
        return torch.pow(x, n)
    return math.pow(x, n)

def sin(x):
    if isinstance(x, torch.Tensor):
        return torch.sin(x)
    return math.sin(x)

def sqrt(x):
    if isinstance(x, torch.Tensor):
        return torch.sqrt(x)
    return math.sqrt(x)

def tan(x):
    if isinstance(x, torch.Tensor):
        return torch.tan(x)
    return math.tan(x)

F = torch.sigmoid
device = torch.device('cuda')

def length(x):
    return len(x)

def cot(x):
    return 1.0 / tan(x)

def fabs(x):
    return abs(x)

)";

constexpr auto writeBackFunc = R"(

    def write_back(self, action):
        table = {}
        for vars in table[action]:
            self.cached_vars[vars] = self.temp_cached_vars[vars]
)";

} // namespace

namespace rulejit::pybe {

void PYEngine::buildFromSource(const std::string& srcXML) {

    using namespace rulejit::ruleset;

    // discard statements in preDefines
    // TODO: execute preDefines once(in RuleSet::Init()) to handle init value?
    auto [preDefines, preProcess, subRuleSets] = RuleSetParser::readSource(srcXML, context, data);
    subRuleSets.emplace(subRuleSets.begin(), std::move(preProcess[0]));
    context.scope.begin()->varDef.clear();

    if (!outputPath.ends_with('/')) {
        outputPath.push_back('/');
    }
    if (!std::filesystem::exists(outputPath)) {
        std::filesystem::create_directories(outputPath);
    }

    
    std::ofstream headerFile(std::format("{}header.py", outputPath));
    headerFile << pyFileHeader;
    // collect subruleset defs
    for (auto&& [id, astName] : std::views::enumerate(subRuleSets)) {
        auto& ast = context.global.realFuncDefinition[astName]->returnValue;
        std::vector<std::string> members;
        for (auto&& [atom, cacheNames] : std::views::enumerate(data.modifiedValue[id])) {
            auto modified = cacheNames | std::views::filter([&](const std::string& n) {
                                return std::ranges::find(data.cacheVar, n) != data.cacheVar.end();
                            }) |
                            std::views::transform([](auto& s) { return "\"" + s + "\""; }) |
                            std::ranges::to<std::vector>();
            members.push_back(std::format("[{}]", modified | tools::mystr::join(", ")));
        }
        std::string code = codegen.gen(ast, id);
        std::string writeBacks = std::format(writeBackFunc, std::format("[{}]", members | tools::mystr::join(", ")));
        std::ofstream subRuleSetFile(std::format("{}subruleset{}.py", outputPath, id));
        subRuleSetFile << "from header import *\n\n" << code << writeBacks;
    }
}

} // namespace rulejit::pybe