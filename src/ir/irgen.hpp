/**
 * @file irgen.hpp
 * @author djw
 * @brief IR/IR generator
 * @date 2023-03-28
 *
 * @details Includes IRGenerator, a class used to generate IR from AST
 *
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-28</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

#include <memory>

#include "ast/ast.hpp"
#include "ast/astvisitor.hpp"
#include "ast/context.hpp"
#include "ir/irholder.hpp"

namespace rulejit {

/**
 * @brief Class for generate LLVM-IR from AST
 *
 */
struct IRGenerator : public ASTVisitor {
    /**
     * @brief Construct a new IRGenerator object
     *
     * @param holder IRHolder which holds the generated IR
     * @param context ContextStack which provides IR generation context
     */
    IRGenerator(IRHolder &holder, ContextStack &context) : h(holder), c(context) {}
    IRGenerator(const IRGenerator &) = delete;
    IRGenerator(IRGenerator &&) = delete;
    IRGenerator &operator=(const IRGenerator &) = delete;
    IRGenerator &operator=(IRGenerator &&) = delete;

    virtual ~IRGenerator() = default;
    /**
     * @brief Stream operator| used to generate function def in ContextGlobal
     * @attention function must have checked
     *
     * @param src function name need to be generated
     * @param irgen receiver
     */
    void friend operator|(std::string &src, IRGenerator &irgen) {
        if (auto it = context.global.realFuncDefinition.find(src);
            it != context.global.realFuncDefinition.end() && context.global.checkedFunc.contains(it->second)) {
            irgen.generate(it->first, it->second);
        } else {
            throw setError("No such function or maybe unchecked: " + src);
        }
    }
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
    void generate(const std::string &name, std::unique_ptr<FunctionDefAST> &ast) {}
    [[noreturn]] void setError(std::string msg) { throw std::logic_error(msg); }
};

} // namespace rulejit