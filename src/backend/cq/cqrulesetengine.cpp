/**
 * @file cqrulesetengine.cpp
 * @author djw
 * @brief CQ/Interpreter/RuleSetEngine
 * @date 2023-03-27
 *
 * @details
 *
 * TODO: move to src/release
 *
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-27</td><td>Initial version.</td></tr>
 * <tr><td>djw</td><td>2023-03-29</td><td>Move XML-Parsering to frontend/ruleset</td></tr>
 * <tr><td>djw</td><td>2023-04-18</td><td>make every <Value> a single subruleset</td></tr>
 * </table>
 */
#include "cqrulesetengine.h"

#include <iostream>

#include "frontend/ruleset/rulesetparser.h"
#include "rapidxml-1.13/rapidxml.hpp"
#include "tools/seterror.hpp"

namespace rulejit::cq {

void RuleSetEngine::buildFromSource(const std::string &srcXML) {

    using namespace rulejit::rulesetxml;

    // CAUTION: discard statements in preDefines
    // TODO: execute preDefines once to handle init value?
#ifdef __RULEJIT_DEBUG_IN_RUNTIME
    auto [preDefines, preProcess, subRuleSets, tmpDebugInfo] = RuleSetParser::readSource(srcXML, context, dataStorage.metaInfo);
    debugInfo = std::move(tmpDebugInfo);
#else
    auto [preDefines, preProcess, subRuleSets] = RuleSetParser::readSource(srcXML, context, dataStorage.metaInfo);
#endif // __RULEJIT_DEBUG_IN_RUNTIME

    std::set<std::string> notGenerate{preProcess.begin(), preProcess.end()};
    notGenerate.emplace(preDefines);

    for (auto &&name : preProcess) {
        notGenerate.insert(name);
        preprocess.subRuleSets.emplace_back(context, dataStorage);
        preprocess.subRuleSets.back().subruleset = std::move(context.global.realFuncDefinition[name]->returnValue);
    }

    // for each subruleset node, store generated ast in ruleset
    for (auto &&subRuleSetName : subRuleSets) {
        notGenerate.insert(subRuleSetName);
        ruleset.subRuleSets.emplace_back(context, dataStorage);
        auto &tmp = ruleset.subRuleSets.back();
        tmp.subruleset = std::move(context.global.realFuncDefinition[subRuleSetName]->returnValue);
    }

    std::erase_if(context.global.realFuncDefinition, [&](auto &tar) {
        // clear all func that represent a subruleset
        return notGenerate.contains(tar.first);
    });
}

} // namespace rulejit::cq