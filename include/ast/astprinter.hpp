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

    friend std::string operator|(std::unique_ptr<AST> v, ASTPrinter p){
        return p.printAST(v.get(), true);
    }

    // std::map<std::string, std::unique_ptr<DefAST>> context;
    ASTPrinter() = default;
    std::string printAST(AST *ast, bool isIndent = false) {
        if (isIndent) {
            indent = true;
            cnt = 0;
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

    VISIT_FUNCTION(IdentifierExprAST) { 
        buffer << std::format("IDENT[{}]", v.name); 
    }
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
    // VIRTUAL_VISIT_FUNCTION(ComplexLiteralExprAST);
    // VIRTUAL_VISIT_FUNCTION(BranchExprAST);
    // VIRTUAL_VISIT_FUNCTION(LoopAST);
    // VIRTUAL_VISIT_FUNCTION(BlockExprAST);

    // VIRTUAL_VISIT_FUNCTION(TypeDefAST);
    // VIRTUAL_VISIT_FUNCTION(VarDefAST);
    // VIRTUAL_VISIT_FUNCTION(FunctionDefAST);

    // VIRTUAL_VISIT_FUNCTION(TopLevelAST);
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