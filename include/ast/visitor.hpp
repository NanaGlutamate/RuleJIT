#pragma once

#define VISIT_FUNCTION(type) void visit (type &) override
#define VIRTUAL_VISIT_FUNCTION(type) virtual void visit (type &){}
#define PURE_VIRTUAL_VISIT_FUNCTION(type) virtual void visit (type &) = 0

namespace rulejit {

struct IdentifierExprAST;
struct MemberAccessExprAST;
struct LiteralExprAST;
struct FunctionCallExprAST;
struct ComplexLiteralExprAST;

struct VarDefAST;
struct FunctionDefAST;
struct TypeDefAST;

struct Visitor{
    PURE_VIRTUAL_VISIT_FUNCTION(IdentifierExprAST);
    PURE_VIRTUAL_VISIT_FUNCTION(MemberAccessExprAST);
    PURE_VIRTUAL_VISIT_FUNCTION(LiteralExprAST);
    PURE_VIRTUAL_VISIT_FUNCTION(FunctionCallExprAST);
    VIRTUAL_VISIT_FUNCTION(ComplexLiteralExprAST);

    VIRTUAL_VISIT_FUNCTION(VarDefAST);
    VIRTUAL_VISIT_FUNCTION(FunctionDefAST);
    VIRTUAL_VISIT_FUNCTION(TypeDefAST);
    virtual ~Visitor() = default;
};

}