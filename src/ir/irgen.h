#pragma once

#include <memory>

#include "ast/ast.hpp"
#include "ast/astvisitor.hpp"
#include "ast/context.hpp"
#include "ir/irholder.hpp"

namespace rulejit {

struct IRGenerator : public ASTVisitor {
    void loadIRHolder(IRHolder *h) { holder = h; }
    void loadContext(ContextStack *context) { c = context; }
    void friend operator|(std::unique_ptr<ExprAST> &ast, IRGenerator &irgen) {
        if (auto p1 = isType<FunctionDefAST>(ast); p1) {

        } else if (auto p2 = isType<TypeDefAST>(ast); p2) {

        } else if (auto p3 = isType<SymbolCommandAST>(ast); p3) {

        } else {
            // gen unnamed function
        }
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
    VISIT_FUNCTION(FunctionDefAST) {}
    VISIT_FUNCTION(SymbolCommandAST) {}
    virtual ~IRGenerator() = default;

  private:
    IRHolder *holder;
    ContextStack *c;
};

} // namespace rulejit