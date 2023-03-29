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

#include <fstream>
#include <list>

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
    RuleSetEngine() : dataStorage(), ruleset(), context(), preprocess(context, dataStorage) {}
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
    void tick() {
        preprocess.subruleset | preprocess.interpreter;
        preprocess.handler.writeBack();
        preprocess.interpreter.reset();

        for (auto &&s : ruleset.subRuleSets) {
            s.subruleset | s.interpreter;
        }
        for (auto &&s : ruleset.subRuleSets) {
            s.handler.writeBack();
            s.interpreter.reset();
        }
    }

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
    /// @brief data storage
    DataStore dataStorage;
    /// @brief rule set
    RuleSet ruleset;
    /// @brief context
    ContextStack context;
    /// @brief pre-process subruleset, which will called tick() and writeBack() before all subruleset
    SubRuleSet preprocess;
};

} // namespace rulejit::cq