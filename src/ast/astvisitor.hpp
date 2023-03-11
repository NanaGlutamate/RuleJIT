#pragma once

#define VISIT_FUNCTION_DEF(c, type) void c::visit (type & v) override
#define VISIT_FUNCTION(type) void visit (type & v) override
#define VIRTUAL_VISIT_FUNCTION(type) virtual void visit (type & v){unexpectType=true;}
#define PURE_VIRTUAL_VISIT_FUNCTION(type) virtual void visit (type & v) = 0

namespace rulejit {

struct IdentifierExprAST;
struct MemberAccessExprAST;
// struct ArrayIndexExprAST;
struct LiteralExprAST;
struct FunctionCallExprAST;
struct BinOpExprAST;
struct BranchExprAST;
struct ComplexLiteralExprAST;
struct LoopAST;
struct BlockExprAST;

// struct AssignmentAST;

struct ControlFlowAST;

struct TypeDefAST;
struct VarDefAST;
struct FunctionDefAST;

// struct SymbolCommandAST;

// struct TopLevelAST;

// struct Foo : public ASTVisitor{
//     VISIT_FUNCTION(IdentifierExprAST);
//     VISIT_FUNCTION(MemberAccessExprAST);
//     VISIT_FUNCTION(LiteralExprAST);
//     VISIT_FUNCTION(FunctionCallExprAST);
//     VISIT_FUNCTION(BinOpExprAST);
//     VISIT_FUNCTION(BranchExprAST);
//     VISIT_FUNCTION(ComplexLiteralExprAST);
//     VISIT_FUNCTION(LoopAST);
//     VISIT_FUNCTION(BlockExprAST);
//     VISIT_FUNCTION(ControlFlowAST);
//     VISIT_FUNCTION(TypeDefAST);
//     VISIT_FUNCTION(VarDefAST);
//     VISIT_FUNCTION(FunctionDefAST);
// };

struct ASTVisitor{
    bool unexpectType;
    ASTVisitor() = default;
    PURE_VIRTUAL_VISIT_FUNCTION(IdentifierExprAST);
    PURE_VIRTUAL_VISIT_FUNCTION(MemberAccessExprAST);
    // PURE_VIRTUAL_VISIT_FUNCTION(ArrayIndexExprAST);
    PURE_VIRTUAL_VISIT_FUNCTION(LiteralExprAST);
    PURE_VIRTUAL_VISIT_FUNCTION(FunctionCallExprAST);
    PURE_VIRTUAL_VISIT_FUNCTION(BinOpExprAST);
    PURE_VIRTUAL_VISIT_FUNCTION(BranchExprAST);
    VIRTUAL_VISIT_FUNCTION(ComplexLiteralExprAST);
    VIRTUAL_VISIT_FUNCTION(LoopAST);
    VIRTUAL_VISIT_FUNCTION(BlockExprAST);

    // VIRTUAL_VISIT_FUNCTION(AssignmentAST);

    VIRTUAL_VISIT_FUNCTION(ControlFlowAST);

    VIRTUAL_VISIT_FUNCTION(TypeDefAST);
    VIRTUAL_VISIT_FUNCTION(VarDefAST);
    VIRTUAL_VISIT_FUNCTION(FunctionDefAST);

    // VIRTUAL_VISIT_FUNCTION(SymbolCommandAST);

    // VIRTUAL_VISIT_FUNCTION(TopLevelAST);
    virtual ~ASTVisitor() = default;
};

}