#pragma once

#define VISIT_FUNCTION_DEF(c, type) void c::visit (type &) override
#define VISIT_FUNCTION(type) void visit (type &) override
#define VIRTUAL_VISIT_FUNCTION(type) virtual void visit (type &){}
#define PURE_VIRTUAL_VISIT_FUNCTION(type) virtual void visit (type &) = 0

namespace rulejit {

struct IdentifierExprAST;
struct MemberAccessExprAST;
struct LiteralExprAST;
struct FunctionCallExprAST;
struct ComplexLiteralExprAST;
struct BranchExprAST;
struct LoopAST;
struct BlockExprAST;

struct AssignmentAST;

struct TypeDefAST;
struct VarDefAST;
struct FunctionDefAST;

struct TopLevelAST;

struct ASTVisitor{
    PURE_VIRTUAL_VISIT_FUNCTION(IdentifierExprAST);
    PURE_VIRTUAL_VISIT_FUNCTION(MemberAccessExprAST);
    PURE_VIRTUAL_VISIT_FUNCTION(LiteralExprAST);
    PURE_VIRTUAL_VISIT_FUNCTION(FunctionCallExprAST);
    VIRTUAL_VISIT_FUNCTION(ComplexLiteralExprAST);
    VIRTUAL_VISIT_FUNCTION(BranchExprAST);
    VIRTUAL_VISIT_FUNCTION(LoopAST);
    VIRTUAL_VISIT_FUNCTION(BlockExprAST);

    VIRTUAL_VISIT_FUNCTION(AssignmentAST);

    VIRTUAL_VISIT_FUNCTION(TypeDefAST);
    VIRTUAL_VISIT_FUNCTION(VarDefAST);
    VIRTUAL_VISIT_FUNCTION(FunctionDefAST);

    VIRTUAL_VISIT_FUNCTION(TopLevelAST);
    virtual ~ASTVisitor() = default;
};

}