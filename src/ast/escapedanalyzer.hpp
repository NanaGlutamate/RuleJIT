/**
 * @file escapedanalyzer.hpp
 * @author djw
 * @brief AST/Escaped variable analyzer
 * @date 2023-03-28
 * 
 * @details Includes a tool to analyze escaped variables for capture analysis,
 * not used for now
 * 
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-28</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

#include "ast/ast.hpp"
#include "ast/astvisitor.hpp"

namespace rulejit {

struct EscapedVarAnalyzer : public ASTVisitor {
    EscapedVarAnalyzer() = default;
    std::set<std::string> friend operator|(std::unique_ptr<ExprAST> &ast, EscapedVarAnalyzer u) {
        my_assert(bool(ast->type), "input must be AST after semantic check");
        u.stack = {{}};
        u.escaped.clear();
        ast->accept(&u);
        return std::move(u.escaped);
    }
    VISIT_FUNCTION(IdentifierExprAST) {
        for (auto &&scope : stack) {
            if (scope.contains(v.name)) {
                return;
            }
        }
        escaped.emplace(v.name);
    }
    VISIT_FUNCTION(MemberAccessExprAST) {
        v.baseVar->accept(this);
        v.memberToken->accept(this);
    }
    VISIT_FUNCTION(LiteralExprAST) {}
    VISIT_FUNCTION(FunctionCallExprAST) {
        for (auto &arg : v.params) {
            arg->accept(this);
        }
        v.functionIdent->accept(this);
    }
    VISIT_FUNCTION(BinOpExprAST) {
        v.lhs->accept(this);
        v.rhs->accept(this);
    }
    VISIT_FUNCTION(UnaryOpExprAST) {
        v.rhs->accept(this);
    }
    VISIT_FUNCTION(BranchExprAST) {
        v.condition->accept(this);
        v.trueExpr->accept(this);
        v.falseExpr->accept(this);
    }
    VISIT_FUNCTION(ComplexLiteralExprAST) {
        for (auto &[index, value] : v.members) {
            if (index)
                index->accept(this);
            value->accept(this);
        }
    }
    VISIT_FUNCTION(LoopAST) {
        stack.emplace_back();
        v.init->accept(this);
        v.condition->accept(this);
        v.body->accept(this);
        stack.pop_back();
    }
    VISIT_FUNCTION(BlockExprAST) {
        stack.emplace_back();
        for (auto &stmt : v.exprs) {
            stmt->accept(this);
        }
        stack.pop_back();
    }
    VISIT_FUNCTION(ControlFlowAST) {}
    VISIT_FUNCTION(TypeDefAST) {}
    VISIT_FUNCTION(VarDefAST) {
        v.definedValue->accept(this);
        stack.back().emplace(v.name);
    }
    VISIT_FUNCTION(FunctionDefAST) {}
    VISIT_FUNCTION(SymbolDefAST) {}
    virtual ~EscapedVarAnalyzer() = default;

  private:
    std::set<std::string> escaped;
    std::vector<std::set<std::string>> stack;
};

} // namespace rulejit
