/**
 * @file pybe.h
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
#pragma once

#include <fstream>
#include <list>

#include "frontend/lexer.h"
#include "frontend/parser.h"
#include "frontend/ruleset/rulesetparser.h"
#include "frontend/semantic.hpp"
#include "pycodegen.hpp"

namespace rulejit::pybe {

/**
 * @brief main class to generate cpp code from ruleset XML
 * 
 */
struct PYEngine {
    PYEngine() : context(), semantic(context), codegen(context, data) {
        outputPath = "./";
    };
    PYEngine(const PYEngine &) = delete;
    PYEngine(PYEngine &&) = delete;
    PYEngine &operator=(const PYEngine &) = delete;
    PYEngine &operator=(PYEngine &&) = delete;

    /**
     * @brief build from XML file
     * 
     * @param srcXML string of content of XML file
     * @return none.
     */
    void buildFromSource(const std::string &srcXML);

    /**
     * @brief build from XML file
     * 
     * @param srcXML XMLFilePath string of path of XML file
     * @return none.
     */
    void buildFromFile(const std::string &XMLFilePath) {
        using namespace std;
        std::ifstream file(XMLFilePath);
        std::string buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        buildFromSource(buffer);
    }

    std::string outputPath;
  private:
    ruleset::RuleSetMetaInfo data;
    ContextStack context;
    ExpressionLexer lexer;
    ExpressionParser parser;
    ExpressionSemantic semantic;
    PYCodeGen codegen;
};

} // namespace rulejit::cppgen