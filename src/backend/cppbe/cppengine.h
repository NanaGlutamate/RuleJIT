/**
 * @file cppengine.h
 * @author djw
 * @brief CQ/CPPBE/Cpp Engine
 * @date 2023-03-27
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

#include "backend/cppbe/metainfo.hpp"
#include "backend/cppbe/subrulesetgen.hpp"
#include "frontend/lexer.h"
#include "frontend/parser.h"
#include "frontend/ruleset/rulesetparser.h"
#include "frontend/semantic.hpp"

namespace rulejit::cppgen {

/**
 * @brief main class to generate cpp code from ruleset XML
 * 
 */
struct CppEngine {
    CppEngine() : context(), data(), semantic(context), codegen(context, data) {
        namespaceName = "ruleset";
        outputPath = "./src/";
    };
    CppEngine(const CppEngine &) = delete;
    CppEngine(CppEngine &&) = delete;
    CppEngine &operator=(const CppEngine &) = delete;
    CppEngine &operator=(CppEngine &&) = delete;
    void setPrefix(const std::string &p) { prefix = p; }
    void setNamespaceName(const std::string &n) { namespaceName = n; }
    void setOutputPath(const std::string &p) {
        outputPath = p;
        if (outputPath.back() != '/' && outputPath.back() != '\\') {
            outputPath.push_back('/');
        }
    }

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
    std::string prefix, namespaceName, outputPath;

  private:
    rulesetxml::RuleSetMetaInfo data;
    ContextStack context;
    ExpressionLexer lexer;
    ExpressionParser parser;
    ExpressionSemantic semantic;
    SubRuleSetCodeGen codegen;
};

} // namespace rulejit::cppgen