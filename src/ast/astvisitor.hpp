/**
 * @file astvisitor.hpp
 * @author djw
 * @brief AST/AST visitor
 * @date 2023-03-27
 *
 * @details Includes defination of interface ASTVisitor, which is a part of AST design.
 * AST designed in Visitor Pattern, provides flexibility in AST operations developments.
 * also contains marcos to simplify extensiones development.
 *
 * @see ASTVisitor
 *
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-27</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

// #define VISIT_FUNCTION_DEF(c, type) void c::visit(type &v) override
#define VISIT_FUNCTION(type) void visit(type &v) override
#define PURE_VIRTUAL_VISIT_FUNCTION(type) virtual void visit(type &v) = 0

#define AST_FRIEND_DECLEARATION                                                                                        \
    friend struct IdentifierExprAST;                                                                                   \
    friend struct MemberAccessExprAST;                                                                                 \
    friend struct LiteralExprAST;                                                                                      \
    friend struct FunctionCallExprAST;                                                                                 \
    friend struct BinOpExprAST;                                                                                        \
    friend struct UnaryOpExprAST;                                                                                      \
    friend struct BranchExprAST;                                                                                       \
    friend struct ComplexLiteralExprAST;                                                                               \
    friend struct LoopAST;                                                                                             \
    friend struct BlockExprAST;                                                                                        \
    friend struct ControlFlowAST;                                                                                      \
    friend struct TypeDefAST;                                                                                          \
    friend struct VarDefAST;                                                                                           \
    friend struct FunctionDefAST;                                                                                      \
    friend struct SymbolDefAST;

namespace rulejit {

struct IdentifierExprAST;
struct MemberAccessExprAST;
struct LiteralExprAST;
struct FunctionCallExprAST;
struct BinOpExprAST;
struct UnaryOpExprAST;
struct BranchExprAST;
struct ComplexLiteralExprAST;
struct LoopAST;
struct BlockExprAST;

struct ControlFlowAST;

struct TypeDefAST;
struct VarDefAST;
struct FunctionDefAST;

struct SymbolDefAST;

// template

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

/**
 * @ingroup ast
 * @brief Pure virtual interface as part of Visitor Design Pattern
 *
 */
struct ASTVisitor {
    AST_FRIEND_DECLEARATION
    ASTVisitor() = default;
    virtual ~ASTVisitor() = default;

  protected:
    /// @brief visit function for IdentifierExprAST
    /// @param v IdentifierExprAST to visit
    PURE_VIRTUAL_VISIT_FUNCTION(IdentifierExprAST);
    
    /// @brief visit function for MemberAccessExprAST
    /// @param v MemberAccessExprAST to visit
    PURE_VIRTUAL_VISIT_FUNCTION(MemberAccessExprAST);

    /// @brief visit function for LiteralExprAST
    /// @param v LiteralExprAST to visit
    PURE_VIRTUAL_VISIT_FUNCTION(LiteralExprAST);

    /// @brief visit function for FunctionCallExprAST
    /// @param v FunctionCallExprAST to visit
    PURE_VIRTUAL_VISIT_FUNCTION(FunctionCallExprAST);

    /// @brief visit function for BinOpExprAST
    /// @param v BinOpExprAST to visit
    PURE_VIRTUAL_VISIT_FUNCTION(BinOpExprAST);

    /// @brief visit function for UnaryOpExprAST
    /// @param v UnaryOpExprAST to visit
    PURE_VIRTUAL_VISIT_FUNCTION(UnaryOpExprAST);

    /// @brief visit function for BranchExprAST
    /// @param v BranchExprAST to visit
    PURE_VIRTUAL_VISIT_FUNCTION(BranchExprAST);

    /// @brief visit function for ComplexLiteralExprAST
    /// @param v ComplexLiteralExprAST to visit
    PURE_VIRTUAL_VISIT_FUNCTION(ComplexLiteralExprAST);

    /// @brief visit function for LoopAST
    /// @param v LoopAST to visit
    PURE_VIRTUAL_VISIT_FUNCTION(LoopAST);

    /// @brief visit function for BlockExprAST
    /// @param v BlockExprAST to visit
    PURE_VIRTUAL_VISIT_FUNCTION(BlockExprAST);

    /// @brief visit function for ControlFlowAST
    /// @param v ControlFlowAST to visit
    PURE_VIRTUAL_VISIT_FUNCTION(ControlFlowAST);

    /// @brief visit function for TypeDefAST
    /// @param v TypeDefAST to visit
    PURE_VIRTUAL_VISIT_FUNCTION(TypeDefAST);

    /// @brief visit function for VarDefAST
    /// @param v VarDefAST to visit
    PURE_VIRTUAL_VISIT_FUNCTION(VarDefAST);

    /// @brief visit function for FunctionDefAST
    /// @param v FunctionDefAST to visit
    PURE_VIRTUAL_VISIT_FUNCTION(FunctionDefAST);

    /// @brief visit function for SymbolDefAST
    /// @param v SymbolDefAST to visit
    PURE_VIRTUAL_VISIT_FUNCTION(SymbolDefAST);
};

} // namespace rulejit