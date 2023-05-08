/**
 * @file cqrulesetengine.h
 * @author djw
 * @brief CQ/Interpreter/RuleSetEngine
 * @date 2023-03-27
 *
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-27</td><td>Initial version.</td></tr>
 * <tr><td>djw</td><td>2023-03-29</td><td>Add semantic support.</td></tr>
 * </table>
 */
#pragma once

#include "defines/marco.hpp"

#include <array>
#include <fstream>
#include <list>
#include "tools/stringprocess.hpp"
#include <ranges>
#ifdef __RULEJIT_PARALLEL_ENGINE
#include <algorithm>
#include <execution>
#endif // __RULEJIT_PARALLEL_ENGINE

#include "ast/context.hpp"
#include "ast/decompiler.hpp"
#include "backend/cq/cqinterpreter.hpp"
#include "backend/cq/cqresourcehandler.h"

namespace rulejit::cq {

/**
 * @brief Structure for subrule set.
 */
struct SubRuleSet {
    /**
     * @brief Construct a new SubRuleSet object.
     *
     * @param context context which contains function defines.
     * @param dataStorage The DataStore object.
     */
    SubRuleSet(ContextStack &context, DataStore &dataStorage)
        : handler(dataStorage), interpreter(context, handler), subruleset(nullptr) {}
    SubRuleSet() = delete;
    SubRuleSet(const SubRuleSet &) = delete;
    SubRuleSet(SubRuleSet &&) = delete;
    SubRuleSet &operator=(const SubRuleSet &) = delete;
    SubRuleSet &operator=(SubRuleSet &&) = delete;

    /// @brief resource handler
    ResourceHandler handler;
    /// @brief expression interpreter
    CQInterpreter interpreter;
    /// @brief subruleset AST
    std::unique_ptr<ExprAST> subruleset;
};

/**
 * @brief Structure for rule set.
 */
struct RuleSet {
    std::list<SubRuleSet> subRuleSets;
};

/**
 * @brief Structure for rule set engine.
 */
struct RuleSetEngine {
    RuleSetEngine() : dataStorage(), ruleset(), context(), preprocess() {}
    RuleSetEngine(const RuleSetEngine &) = delete;
    RuleSetEngine(RuleSetEngine &&) = delete;
    RuleSetEngine &operator=(const RuleSetEngine &) = delete;
    RuleSetEngine &operator=(RuleSetEngine &&) = delete;

    /**
     * @brief Build the rule set engine from the XML source.
     *
     * @param srcXML The string content of the XML file.
     * @return void.
     */
    void buildFromSource(const std::string &srcXML);

    /**
     * @brief Build the rule set engine from the XML file.
     *
     * @param XMLFilePath The string path of the XML file.
     * @return void.
     */
    void buildFromFile(const std::string &XMLFilePath) {
        using namespace std;
        std::ifstream file(XMLFilePath);
        std::string buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        buildFromSource(buffer);
    }

    /**
     * @brief Initialize the rule set engine.
     *
     * @return void.
     */
    void init() { dataStorage.Init(); }

    /**
     * @brief Execute a tick of the rule set engine.
     *
     * @return void.
     */
    void tick() { execute(); }

    /**
     * @brief Set the input data for the rule set engine.
     *
     * @param input The unordered map of input data.
     * @return void.
     */
    void setInput(const std::unordered_map<std::string, std::any> &input) { dataStorage.SetInput(input); }

    /**
     * @brief get the output data from the rule set engine.
     *
     * @return The pointer of unordered map for output data.
     */
    std::unordered_map<std::string, std::any> *getOutput() { return dataStorage.GetOutput(); }

    /**
     * @brief get the cached data from the rule set engine.
     *
     * @return const std::unordered_map<std::string, std::any>&
     */
    const std::unordered_map<std::string, std::any> &getCache() { return dataStorage.cache; }

    /**
     * @brief get the input data from the rule set engine.
     *
     * @return const std::unordered_map<std::string, std::any>&
     */
    const std::unordered_map<std::string, std::any>& getInput() { return dataStorage.input; }

    std::vector<int> hitRules() {
        std::vector<int> ret;
        for (auto& ruleset : preprocess.subRuleSets) {
            ret.push_back(ruleset.interpreter.getReturned());
        }
        for (auto& ruleset : ruleset.subRuleSets) {
            ret.push_back(ruleset.interpreter.getReturned());
        }
        return ret;
    }

  private:
    void execute() {
        for (auto &ruleset : std::array<RuleSet *, 2>{&preprocess, &ruleset}) {
#ifdef __RULEJIT_PARALLEL_ENGINE
            std::foreach (std::execution::par_unseq, ruleset->subRuleSets.begin(), ruleset->subRuleSets.end(),
                          [](auto &s) { s.subruleset | s.interpreter; });
#else // __RULEJIT_PARALLEL_ENGINE
            size_t cnt = 0;
            for (auto &s : ruleset->subRuleSets) {
                try {
                    try {
                        s.subruleset | s.interpreter;
                    } catch (std::logic_error &e) {
                        throw e;
                    } catch (...) {
                        error("[Unhandled Exception]");
                    }
                } catch (std::logic_error &e) {
                    using namespace std::views;
                    using namespace std::literals;
                    using namespace tools::mystr;
                    Decompiler decompiler;
                    std::string name = ruleset == &preprocess ? "pre processing"
                                                              : "sub ruleset " + std::to_string(cnt) + "(zero-based)";
                    std::string info = e.what() + "\n\nin "s + name + " when try to execute expression\n";
                    info += "decompiled context:\n";
                    for (auto p : s.interpreter.currentExpr | reverse | take(7)) {
                        info += ("    at context(decompiled): "s + decompiler.decompile(p) + "\n");
                    }
                    info += "Core dump: \n\n";
                    info += dataStorage.dump();
                    auto typeCheck = dataStorage.genTypeCheckInfo();
                    if (!typeCheck.empty()) {
                        info += "Type Check info: \n\n";
                        info += std::move(typeCheck);
                    }
                    error(info);
                }
                cnt++;
            }
#endif // __RULEJIT_PARALLEL_ENGINE
            for (auto &s : ruleset->subRuleSets) {
                s.handler.writeBack();
                s.interpreter.reset();
            }
        }
    }

    /// @brief data storage
    DataStore dataStorage;
    /// @brief rule set
    RuleSet ruleset;
    /// @brief context
    ContextStack context;
    /// @brief pre-process subruleset, which will called tick() and writeBack() before all subruleset
    RuleSet preprocess;
};

} // namespace rulejit::cq