#pragma once

#include <memory>

#include "ast/ast.hpp"
#include "ast/astvisitor.hpp"
#include "ast/context.hpp"
#include "ir/irholder.hpp"

namespace rulejit {

struct IRGenerator : public ASTVisitor {
    IRGenerator(IRHolder &holder, ContextStack &context):h(holder), c(context){}
    IRGenerator(const IRGenerator &) = delete;
    IRGenerator(IRGenerator &&) = delete;
    IRGenerator &operator=(const IRGenerator &) = delete;
    IRGenerator &operator=(IRGenerator &&) = delete;
    virtual ~IRGenerator() = default;
    void clear() {}
    void friend operator|(std::pair<std::string, std::unique_ptr<FunctionDefAST>> &src, IRGenerator &irgen) {
        irgen.generate(src.first, src.second);
    }
    void generate(const std::string &name, std::unique_ptr<FunctionDefAST> &ast) {}
    VISIT_FUNCTION(IdentifierExprAST) {}
    VISIT_FUNCTION(MemberAccessExprAST) {}
    VISIT_FUNCTION(LiteralExprAST) {}
    VISIT_FUNCTION(FunctionCallExprAST) {}
    VISIT_FUNCTION(BinOpExprAST) {}
    VISIT_FUNCTION(UnaryOpExprAST) {}
    VISIT_FUNCTION(BranchExprAST) {}
    VISIT_FUNCTION(ComplexLiteralExprAST) {}
    VISIT_FUNCTION(LoopAST) {}
    VISIT_FUNCTION(BlockExprAST) {}
    VISIT_FUNCTION(ControlFlowAST) {}
    VISIT_FUNCTION(TypeDefAST) {}
    VISIT_FUNCTION(VarDefAST) {}
    VISIT_FUNCTION(FunctionDefAST) {}
    VISIT_FUNCTION(SymbolDefAST) {}

  private:
    IRHolder &h;
    ContextStack &c;
    std::set<std::string> generatedFunc;
    std::set<std::string> generatedVar;
};

} // namespace rulejit