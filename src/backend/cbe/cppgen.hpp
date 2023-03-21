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
#include "tools/myassert.hpp"

#define __RULEJIT_SEMANTIC_PUSH pushStack(v.get());
#define __RULEJIT_SEMANTIC_POP popStack();

namespace rulejit {

struct CppCodeGen : public ASTVisitor {
    CppCodeGen() = default;
    std::string prefix;
    std::string namespaceName;
    void setPrefix(const std::string &p) { prefix = p; }
    void setNamespaceName(const std::string &n) { namespaceName = n; }
    void loadContext(ContextStack &context) { c = &context; }
    struct file {
        std::string name;
        std::string content;
    };
    std::vector<file> friend operator|(std::vector<std::unique_ptr<ExprAST>> ast, CppCodeGen &t) {
        std::vector<file> ret;
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
        file ret{prefix + "typedef.hpp", ""};
        std::string buffer;
        for(auto&& [name, type] : c->global.typeDef){
            // TODO: buffer << std::format(typeDef, );
            std::string member, serialize, deserialize;
            for(size_t i = 0; i < type.subTypes.size(); ++i){
                member += std::format(typeMember, CppStyleType(type.subTypes[i]), type.idents[i+1]);
                serialize += std::format(typeSerialize, CppStyleType(type.subTypes[i]), type.idents[i+1]);
                deserialize += std::format(typeDeserialize, CppStyleType(type.subTypes[i]), type.idents[i+1]);
            }
            buffer += std::
        }
    };
    std::string CppStyleType(const TypeInfo& type){
        // only support vector and base type
        if(type.isBaseType()){
            return type.idents[0];
        }
    }
    [[noreturn]] void setError(const std::string &info,
                               const std::source_location location = std::source_location::current()) {

        throw std::logic_error(std::format("Code Generate Error in {}::{}, line{}: {}", location.file_name(),
                                           location.function_name(), location.line(), info));
        // return nullptr;
    }
    ContextStack *c;
};

} // namespace rulejit