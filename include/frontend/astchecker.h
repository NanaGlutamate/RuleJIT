#pragma once

#include "ast/visitor.hpp"

namespace rulejit {

struct ASTChecker : public Visitor {
    VISIT_FUNCTION(IdentifierExprAST);
    VISIT_FUNCTION(LiteralExprAST);
    VISIT_FUNCTION(FunctionCallExprAST);
    VISIT_FUNCTION(VarDefAST);
    VISIT_FUNCTION(FunctionDefAST);
    VISIT_FUNCTION(TypeDefAST);
};

}