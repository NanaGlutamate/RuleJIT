#pragma once

#include <format>
#include <map>
#include <memory>
#include <ranges>
#include <source_location>
#include <sstream>
#include <string>

#include "ast/ast.hpp"
#include "ast/astvisitor.hpp"
#include "ast/context.hpp"
#include "backend/cbe/template.hpp"
#include "backend/cbe/metainfo.hpp"
#include "tools/myassert.hpp"

namespace rulejit::cppgen {

struct CppCodeGen : public ASTVisitor {
    CppCodeGen() = default;
    std::string prefix;
    std::string namespaceName;
    void setPrefix(const std::string &p) { prefix = p; }
    void setNamespaceName(const std::string &n) { namespaceName = n; }
    void loadContext(ContextStack &context) { c = &context; }
    void loadMetaInfo(MetaInfo &metaInfo) { m = &metaInfo; }
    struct file {
        std::string name;
        std::string content;
    };
    std::vector<file> friend operator|(std::vector<std::unique_ptr<ExprAST>> ast, CppCodeGen &t) {
        std::vector<file> ret;
        ret.push_back(t.genTypeDef());
    }
    VISIT_FUNCTION(IdentifierExprAST) {}
    VISIT_FUNCTION(MemberAccessExprAST) {}
    VISIT_FUNCTION(LiteralExprAST) {}
    VISIT_FUNCTION(FunctionCallExprAST) {}
    VISIT_FUNCTION(BinOpExprAST) {}
    VISIT_FUNCTION(BranchExprAST) {}
    VISIT_FUNCTION(ComplexLiteralExprAST) {}
    VISIT_FUNCTION(LoopAST) {}
    VISIT_FUNCTION(BlockExprAST) {}
    VISIT_FUNCTION(ControlFlowAST) {}
    VISIT_FUNCTION(TypeDefAST) {}
    VISIT_FUNCTION(VarDefAST) {}
    VISIT_FUNCTION(FunctionDefAST) { return setError("FunctionDefAST not supported"); }
    VISIT_FUNCTION(SymbolCommandAST) { return setError("SymbolCommandAST not supported"); }

  private:
    file genTypeDef() {
        using namespace templates;
        std::string buffer;
        for(auto&& [name, type] : c->global.typeDef){
            // TODO: buffer << std::format(typeDef, );
            std::string member, serialize, deserialize;
            if(type.subTypes.size() + 1 != type.idents.size() || type.idents[0] != "struct"){
                setError(std::format("unsupported type: {}", type.toString()));
            }
            for(size_t i = 0; i < type.subTypes.size(); ++i){
                member += std::format(typeMember, CppStyleType(type.subTypes[i]), type.idents[i+1]);
                serialize += std::format(typeSerialize, CppStyleType(type.subTypes[i]), type.idents[i+1]);
                deserialize += std::format(typeDeserialize, CppStyleType(type.subTypes[i]), type.idents[i+1]);
            }
            buffer += std::format(typeDef, name, member, deserialize, serialize);
        }
        return {prefix + "typedef.hpp", std::format(typeDefHpp, namespaceName, prefix, buffer)};
    };
    std::string CppStyleType(const TypeInfo& type){
        // only support vector and base type
        if(type.isBaseType()){
            if(type.idents[0] == "f64")return "double";
            return type.idents[0];
        }
        if(type.idents.size() == 2 && type.idents[0] == "[]"){
            return "std::vector<" + type.idents[1] + ">";
        }
        setError(std::format("unsupported type: {}", type.toString()));
    }
    [[noreturn]] void setError(const std::string &info,
                               const std::source_location location = std::source_location::current()) {

        throw std::logic_error(std::format("Code Generate Error in {}::{}, line{}: {}", location.file_name(),
                                           location.function_name(), location.line(), info));
        // return nullptr;
    }
    ContextStack *c;
    MetaInfo *m;
};

} // namespace rulejit