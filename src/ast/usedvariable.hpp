#pragma once

#include "ast/astvisitor.hpp"
#include "ast/ast.hpp"

namespace rulejit{

struct UsedVariable : public ASTVisitor{
    UsedVariable() = default;
    std::set<std::string> friend operator|(std::unique_ptr<ExprAST>& ast, UsedVariable u){
        u.used.clear();
        ast->accept(&u);
        return std::move(u.used);
    }
    VISIT_FUNCTION(IdentifierExprAST){
        used.emplace(v.name);
    }
    VISIT_FUNCTION(MemberAccessExprAST){
        v.baseVar->accept(this);
        v.memberToken->accept(this);
    }
    VISIT_FUNCTION(LiteralExprAST){}
    VISIT_FUNCTION(FunctionCallExprAST){
        for(auto & arg : v.params){
            arg->accept(this);
        }
        v.functionIdent->accept(this);
    }
    VISIT_FUNCTION(BinOpExprAST){
        v.lhs->accept(this);
        v.rhs->accept(this);
    }
    VISIT_FUNCTION(BranchExprAST){
        v.condition->accept(this);
        v.trueExpr->accept(this);
        v.falseExpr->accept(this);
    }
    VISIT_FUNCTION(ComplexLiteralExprAST){
        for(auto & [index, value] : v.members){
            if(index)index->accept(this);
            value->accept(this);
        }
    }
    VISIT_FUNCTION(LoopAST){
        v.init->accept(this);
        v.condition->accept(this);
        v.body->accept(this);
    }
    VISIT_FUNCTION(BlockExprAST){
        for(auto & stmt : v.exprs){
            stmt->accept(this);
        }
    }
    VISIT_FUNCTION(ControlFlowAST){}
    VISIT_FUNCTION(TypeDefAST){}
    VISIT_FUNCTION(VarDefAST){
        v.definedValue->accept(this);
    }
    VISIT_FUNCTION(FunctionDefAST){}
    VISIT_FUNCTION(SymbolDefAST){}
    virtual ~UsedVariable() = default;
private:
    std::set<std::string> used;
};

} // namespace rulejit
