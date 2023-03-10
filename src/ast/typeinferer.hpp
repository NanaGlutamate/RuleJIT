#pragma once

#include <map>
#include <string>
#include <memory>

#include "ast/ast.hpp"
#include "ast/astvisitor.hpp"

namespace rulejit {

struct TypeInferer : public ASTVisitor{
    bool unexpectType;
    ASTVisitor() = default;
    VISIT_FUNCTION(IdentifierExprAST);
    VISIT_FUNCTION(MemberAccessExprAST);
    VISIT_FUNCTION(LiteralExprAST);
    VISIT_FUNCTION(FunctionCallExprAST);
    VISIT_FUNCTION(BranchExprAST);
    VISIT_FUNCTION(ComplexLiteralExprAST);
    VISIT_FUNCTION(LoopAST);
    VISIT_FUNCTION(BlockExprAST);

    VISIT_FUNCTION(ControlFlowAST);

    VISIT_FUNCTION(TypeDefAST);
    VISIT_FUNCTION(VarDefAST);
    VISIT_FUNCTION(FunctionDefAST);

    // VIRTUAL_VISIT_FUNCTION(SymbolCommandAST);

    // VIRTUAL_VISIT_FUNCTION(TopLevelAST);
    virtual ~ASTVisitor() = default;
};

}