/**
 * @file templateins.hpp
 * @author djw
 * @brief
 * @date 2023-04-20
 *
 * @details Contains a class to instantiate templates def.
 *
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-04-20</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

#include "ast/ast.hpp"
#include "ast/astvisitor.hpp"

namespace rulejit {

// TODO: user defined local type has same name with template parameter?
struct TemplateInstantiator : public ASTVisitor {
    template <typename T> TemplateInstantiator(T &&tparam) : tparam(std::forward<T>(tparam)){};
    friend void operator|(std::unique_ptr<ExprAST> &ast, TemplateInstantiator &t) { ast->accept(&t); }
    virtual ~TemplateInstantiator() = default;

  protected:
    VISIT_FUNCTION(IdentifierExprAST) {
        if (v.type) {
            *(v.type) = *(v.type) | tparam;
        }
    }
    VISIT_FUNCTION(MemberAccessExprAST) {
        if (v.type) {
            *(v.type) = *(v.type) | tparam;
        }
        v.baseVar->accept(this);
        v.memberToken->accept(this);
    }
    VISIT_FUNCTION(LiteralExprAST) {
        if (v.type) {
            *(v.type) = *(v.type) | tparam;
        }
    }
    VISIT_FUNCTION(FunctionCallExprAST) {
        if (v.type) {
            *(v.type) = *(v.type) | tparam;
        }
        v.functionIdent->accept(this);
        for (auto &arg : v.params) {
            arg->accept(this);
        }
    }
    VISIT_FUNCTION(BinOpExprAST) {
        if (v.type) {
            *(v.type) = *(v.type) | tparam;
        }
        v.lhs->accept(this);
        v.rhs->accept(this);
    }
    VISIT_FUNCTION(UnaryOpExprAST) {
        if (v.type) {
            *(v.type) = *(v.type) | tparam;
        }
        v.rhs->accept(this);
    }
    VISIT_FUNCTION(BranchExprAST) {
        if (v.type) {
            *(v.type) = *(v.type) | tparam;
        }
        v.condition->accept(this);
        v.trueExpr->accept(this);
        v.falseExpr->accept(this);
    }
    VISIT_FUNCTION(ComplexLiteralExprAST) {
        if (v.type) {
            *(v.type) = *(v.type) | tparam;
        }
        for (auto &[index, value] : v.members) {
            if (index) {
                index->accept(this);
            }
            value->accept(this);
        }
    }
    VISIT_FUNCTION(LoopAST) {
        if (v.type) {
            *(v.type) = *(v.type) | tparam;
        }
        v.init->accept(this);
        v.condition->accept(this);
        v.body->accept(this);
    }
    VISIT_FUNCTION(BlockExprAST) {
        if (v.type) {
            *(v.type) = *(v.type) | tparam;
        }
        for (auto &stmt : v.exprs) {
            stmt->accept(this);
        }
    }
    VISIT_FUNCTION(ControlFlowAST) {}
    VISIT_FUNCTION(TypeDefAST) { setError("Do not support type def in template"); }
    VISIT_FUNCTION(VarDefAST) {
        if (v.type) {
            *(v.type) = *(v.type) | tparam;
        }
        *(v.valueType) = *(v.valueType) | tparam;
        v.definedValue->accept(this);
    }
    VISIT_FUNCTION(FunctionDefAST) {
        if (v.type) {
            *(v.type) = *(v.type) | tparam;
        }
        *(v.funcType) = *(v.funcType) | tparam;
        for (auto &param : v.params) {
            param->accept(this);
        }
        v.returnValue->accept(this);
    }
    VISIT_FUNCTION(SymbolDefAST) { setError("Do not support symbol def in template"); }

  private:
    SET_ERROR_MEMBER("Template Instantiation", void)
    TypeInfo::TemplateParam tparam;
};

} // namespace rulejit
