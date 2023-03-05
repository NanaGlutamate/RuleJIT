#pragma once

#include <map>
#include <string>
#include <memory>

#include "ast/ast.hpp"
#include "ast/astvisitor.hpp"

namespace rulejit {

struct ASTChecker : public ASTVisitor {
    bool output;
    // std::map<std::string, std::unique_ptr<DefAST>> context;
    ASTChecker():output(true){};
    VISIT_FUNCTION(IdentifierExprAST);
    VISIT_FUNCTION(MemberAccessExprAST);
    VISIT_FUNCTION(LiteralExprAST);
    VISIT_FUNCTION(FunctionCallExprAST);
    // VIRTUAL_VISIT_FUNCTION(ComplexLiteralExprAST);
    // VIRTUAL_VISIT_FUNCTION(BranchExprAST);
    // VIRTUAL_VISIT_FUNCTION(LoopAST);
    // VIRTUAL_VISIT_FUNCTION(BlockExprAST);

    // VIRTUAL_VISIT_FUNCTION(TypeDefAST);
    // VIRTUAL_VISIT_FUNCTION(VarDefAST);
    // VIRTUAL_VISIT_FUNCTION(FunctionDefAST);

    // VIRTUAL_VISIT_FUNCTION(TopLevelAST);
};

}