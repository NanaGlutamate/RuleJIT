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
#ifdef __RULEJIT_DEBUG_IN_RUNTIME
#include "tools/stringprocess.hpp"
#include <ranges>
#endif // __RULEJIT_DEBUG_IN_RUNTIME
#ifdef __RULEJIT_PARALLEL_ENGINE
#include <algorithm>
#include <execution>
#endif // __RULEJIT_PARALLEL_ENGINE

#include "ast/context.hpp"
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
     * @brief Get the output data from the rule set engine.
     *
     * @return The pointer of unordered map for output data.
     */
    std::unordered_map<std::string, std::any> *getOutput() { return dataStorage.GetOutput(); }

  private:
    void execute() {
        for (auto &ruleset : std::array<RuleSet *, 2>{&preprocess, &ruleset}) {
#ifdef __RULEJIT_PARALLEL_ENGINE
            std::foreach (std::execution::par_unseq, ruleset->subRuleSets.begin(), ruleset->subRuleSets.end(),
                          [](auto &s) { s.subruleset | s.interpreter; });
#else // __RULEJIT_PARALLEL_ENGINE
#ifdef __RULEJIT_DEBUG_IN_RUNTIME
            size_t cnt = 0;
#endif // __RULEJIT_DEBUG_IN_RUNTIME
            for (auto &s : ruleset->subRuleSets) {
#ifdef __RULEJIT_DEBUG_IN_RUNTIME
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
                    std::string name = ruleset == &preprocess ? "pre processing"
                                                              : "sub ruleset " + std::to_string(cnt) + "(zero-based)";
                    std::string info = e.what() + "\n\nin "s + name + " when try to execute expression:    ";
                    for (auto p : s.interpreter.currentExpr | reverse |
                                      filter([&](auto curr) { return debugInfo[cnt].contains(curr); }) | take(5)) {
                        info +=
                            ("    at context: "s + debugInfo[cnt][p] |
                             transform([](char c) { return c == '\n' ? std::string("\\n") : ""s + c; }) | tools::mystr::join("")) +
                            "\n";
                        if (debugInfo[cnt][p].back() == '\n' || debugInfo[cnt][p].back() == ';') {
                            break;
                        }
                    }
                    info += "Core dump: \n\n";
                    info += dataStorage.dump();
                    info += "Type Check info: \n\n";
                    info += dataStorage.genTypeCheckInfo();
                    error(info);
                }
                cnt++;
#else  // __RULEJIT_DEBUG_IN_RUNTIME
                s.subruleset | s.interpreter;
#endif // __RULEJIT_DEBUG_IN_RUNTIME
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

#ifdef __RULEJIT_DEBUG_IN_RUNTIME
    std::vector<std::map<ExprAST *, std::string>> debugInfo;
#endif // __RULEJIT_DEBUG_IN_RUNTIME
};

} // namespace rulejit::cq