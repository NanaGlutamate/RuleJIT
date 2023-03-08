#pragma once

#include <format>
#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "ast/ast.hpp"
#include "ast/astvisitor.hpp"

namespace rulejit {

struct ASTPrinter : public ASTVisitor {
    std::stringstream buffer;
    bool indent;
    size_t cnt;

    friend std::string operator|(std::unique_ptr<AST> &v, ASTPrinter &p) { return p.printAST(v.get(), true); }

    // std::map<std::string, std::unique_ptr<DefAST>> context;
    ASTPrinter() = default;
    std::string printAST(AST *ast, bool isIndent = false) {
        buffer.clear();
        if (isIndent) {
            indent = true;
            cnt = 0;
        }else{
            indent = false;
        }
        unexpectType = false;
        ast->accept(this);
        std::string out;
        std::string tmp;
        while (std::getline(buffer, tmp)) {
            out += tmp + '\n';
        }
        return out;
    }

    VISIT_FUNCTION(IdentifierExprAST) { buffer << std::format("IDENT[{}]", v.name); }
    VISIT_FUNCTION(MemberAccessExprAST) {
        buffer << "(";
        buffer << "memberAccess, ";
        cnt++;
        printIndent();
        v.baseVar->accept(this);
        buffer << ", ";
        printIndent();
        v.memberToken->accept(this);
        cnt--;
        printIndent();
        buffer << ")";
    }
    VISIT_FUNCTION(LiteralExprAST) { buffer << v.value; }
    VISIT_FUNCTION(FunctionCallExprAST) {
        buffer << "(";
        v.functionIdent->accept(this);
        cnt++;
        for (auto &value : v.params) {
            buffer << ", ";
            printIndent();
            value->accept(this);
        }
        cnt--;
        printIndent();
        buffer << ")";
    }
    VISIT_FUNCTION(BranchExprAST) {
        buffer << "(branch, ";
        cnt++;
        printIndent();
        v.condition->accept(this);
        buffer << ", ";
        printIndent();
        v.trueExpr->accept(this);
        buffer << ", ";
        printIndent();
        v.falseExpr->accept(this);
        cnt--;
        printIndent();
        buffer << ")";
    }
    VISIT_FUNCTION(ComplexLiteralExprAST) {
        buffer << "(" << v.type->toString();
        cnt++;
        for (auto &[key, value] : v.members) {
            buffer << ", ";
            printIndent();
            buffer << "(\"" << key << "\", ";
            value->accept(this);
            buffer << ")";
        }
        cnt--;
        printIndent();
        buffer << ")";
    }
    VISIT_FUNCTION(LoopAST) {
        buffer << "(loop, ";
        cnt++;
        printIndent();
        v.condition->accept(this);
        buffer << ", ";
        printIndent();
        v.body->accept(this);
        cnt--;
        printIndent();
        buffer << ")";
    }
    VISIT_FUNCTION(BlockExprAST) {
        buffer << "(block";
        cnt++;
        for (auto &value : v.preStatement) {
            buffer << ", ";
            printIndent();
            value->accept(this);
        }
        buffer << ", ";
        printIndent();
        v.value->accept(this);
        cnt--;
        printIndent();
        buffer << ")";
    }
    VISIT_FUNCTION(AssignmentAST) {
        buffer << "(assign, ";
        cnt++;
        printIndent();
        v.target->accept(this);
        buffer << ", ";
        printIndent();
        v.value->accept(this);
        cnt--;
        printIndent();
        buffer << ")";
    }
    VISIT_FUNCTION(TypeDefAST) {
        buffer << "(typeDef, ";
        cnt++;
        printIndent();
        buffer << v.name;
        buffer << ", ";
        printIndent();
        buffer << v.type->toString();
        cnt--;
        printIndent();
        buffer << ")";
    }
    VISIT_FUNCTION(VarDefAST) {
        buffer << "(varDef, ";
        cnt++;
        printIndent();
        buffer << v.name;
        buffer << ", ";
        printIndent();
        buffer << v.type->toString();
        buffer << ", ";
        printIndent();
        v.defineValue->accept(this);
        cnt--;
        printIndent();
        buffer << ")";
    }
    VISIT_FUNCTION(FunctionDefAST) {
        buffer << "(funcDef, ";
        cnt++;
        printIndent();
        buffer << v.name;
        buffer << ", ";
        printIndent();
        buffer << v.type->toString();
        buffer << ", ";
        printIndent();
        buffer << "(";
        for (size_t i = 0; i < v.params.size(); i++) {
            buffer << v.params[i]->name;
            if (i < v.params.size() - 1)
                buffer << ", ";
        }
        buffer << ")";
        buffer << ", ";
        printIndent();
        v.returnValue->accept(this);
        cnt--;
        printIndent();
        buffer << ")";
    }
    VISIT_FUNCTION(TopLevelAST){
        buffer << "[toplevel]";
    }

  private:
    void printIndent() {
        if (indent) {
            buffer << '\n';
            for (size_t i = 0; i < cnt; i++) {
                buffer << "    ";
            }
        }
    }
};

} // namespace rulejit