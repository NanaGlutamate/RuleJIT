/**
 * @file decompiler.hpp
 * @author djw
 * @brief AST/AST Decompiler
 * @date 2023-03-28
 * 
 * @details Includes tool to rebuildes source code from AST
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

struct Decompiler : public ASTVisitor {
    Decompiler() = default;
    std::string friend operator|(std::unique_ptr<ExprAST> &ast, const Decompiler& _) {
        static Decompiler u;
        u.returned.clear();
        ast->accept(&u);
        return u.returned;
    }
    
  protected:
    VISIT_FUNCTION(IdentifierExprAST) { returned += v.name; }
    VISIT_FUNCTION(MemberAccessExprAST) {
        v.baseVar->accept(this);
        returned += "[";
        v.memberToken->accept(this);
        returned += "]";
    }
    VISIT_FUNCTION(LiteralExprAST) {
        if (*(v.type) == StringType) {
            returned += "\"";
            returned += v.value;
            returned += "\"";
        } else {
            returned += v.value;
        }
    }
    VISIT_FUNCTION(FunctionCallExprAST) {
        v.functionIdent->accept(this);
        returned += "(";
        for (auto &arg : v.params) {
            arg->accept(this);
            returned += ", ";
        }
        if (returned.back() != '(') {
            returned.erase(returned.end() - 2, returned.end());
        }
        returned += ")";
    }
    VISIT_FUNCTION(BinOpExprAST) {
        returned += "(";
        v.lhs->accept(this);
        returned += " " + v.op + " ";
        v.rhs->accept(this);
        returned += ")";
    }
    VISIT_FUNCTION(UnaryOpExprAST) {
        returned += v.op;
        returned += "(";
        v.rhs->accept(this);
        returned += ")";
    }
    VISIT_FUNCTION(BranchExprAST) {
        returned += "if(";
        v.condition->accept(this);
        returned += ") ";
        v.trueExpr->accept(this);
        returned += " else ";
        v.falseExpr->accept(this);
    }
    VISIT_FUNCTION(ComplexLiteralExprAST) {
        returned += v.type->toString();
        returned += "{";
        for (auto &[index, value] : v.members) {
            if (index) {
                index->accept(this);
                returned += ": ";
            }
            value->accept(this);
            returned += ", ";
        }
        if (returned.back() != '{') {
            returned.erase(returned.end() - 2, returned.end());
        }
        returned += "}";
    }
    VISIT_FUNCTION(LoopAST) {
        returned += "for(";
        v.init->accept(this);
        returned += "; ";
        v.condition->accept(this);
        returned += "; )";
        v.body->accept(this);
    }
    VISIT_FUNCTION(BlockExprAST) {
        returned += "{";
        for (auto &stmt : v.exprs) {
            stmt->accept(this);
            returned += "; ";
        }
        returned += "}";
    }
    VISIT_FUNCTION(ControlFlowAST) { returned += "[CONTROL FLOW]"; }
    VISIT_FUNCTION(TypeDefAST) { returned += "[TYPE DEF]"; }
    VISIT_FUNCTION(VarDefAST) {
        returned += "var ";
        returned += v.type->toString();
        returned += " ";
        returned += v.name;
        returned += " = ";
        v.definedValue->accept(this);
    }
    VISIT_FUNCTION(FunctionDefAST) { returned += "[FUNC DEF]"; }
    VISIT_FUNCTION(SymbolDefAST) { returned += "[SYMBOL DEF]"; }
    virtual ~Decompiler() = default;

  private:
    std::string returned;
};

} // namespace rulejit
