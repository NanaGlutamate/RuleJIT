#pragma once

#define VISIT_FUNCTION(type) void visit (type & )
#define PURE_VIRTUAL_VISIT_FUNCTION(type) virtual void visit (type & ) = 0

namespace rulejit {

struct IdentifierExprAST;
struct LiteralExprAST;
struct FunctionCallExprAST;
struct VarDefAST;
struct FunctionDefAST;

struct Visitor{
    PURE_VIRTUAL_VISIT_FUNCTION(IdentifierExprAST);
    PURE_VIRTUAL_VISIT_FUNCTION(LiteralExprAST);
    PURE_VIRTUAL_VISIT_FUNCTION(FunctionCallExprAST);
    PURE_VIRTUAL_VISIT_FUNCTION(VarDefAST);
    PURE_VIRTUAL_VISIT_FUNCTION(FunctionDefAST);
    virtual ~Visitor() = default;
};


}