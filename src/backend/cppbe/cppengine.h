#pragma once

#include <fstream>
#include <list>

#include "frontend/lexer.h"
#include "frontend/parser.h"
#include "frontend/semantic.hpp"
#include "backend/cppbe/metainfo.hpp"
#include "backend/cppbe/template.hpp"
#include "backend/cppbe/subrulesetgen.hpp"

namespace rulejit::cppgen {

struct CppEngine {
    CppEngine():stack(){
        semantic.loadContext(stack);
        codegen.loadContext(stack);
        codegen.loadMetaInfo(data);
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
    //! build from XML file
    //! @param srcXML string of content of XML file
    //! @return none.
    void buildFromSource(const std::string &srcXML);

    //! build from XML file
    //! @param XMLFilePath string of path of XML file
    //! @return none.
    void buildFromFile(const std::string &XMLFilePath) {
        using namespace std;
        std::ifstream file(XMLFilePath);
        std::string buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        buildFromSource(buffer);
    }
    std::string prefix, namespaceName, outputPath;
private:
    MetaInfo data;
    ContextStack stack;
    ExpressionLexer lexer;
    ExpressionParser parser;
    ExpressionSemantic semantic;
    SubRuleSetCodeGen codegen;
};

} // namespace rulejit::cq