/**
 * @file astprinter.hpp
 * @author djw
 * @brief AST/AST printer
 * @date 2023-03-27
 *
 * @details Includes a simple example implementing the AST Visitor, the ASTPrinter.
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

#include <format>
#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "ast/ast.hpp"
#include "ast/astvisitor.hpp"

namespace rulejit {

/**
 * @ingroup ast
 * @brief class to print ast structure
 *
 */
struct ASTPrinter : public ASTVisitor {
    /**
     * @brief pipe operator| provides function to print given ast
     *
     * @param v given ast
     * @param p operator receiver
     * @return std::string printed string
     */
    friend std::string operator|(std::unique_ptr<ExprAST> &v, ASTPrinter &p) { return p.printAST(v.get(), true); }

    /**
     * @brief Construct a new ASTPrinter object
     *
     */
    ASTPrinter() = default;
    /**
     * @brief print given ast. same as operator|, but provides more detailed control like is indent
     *
     * @param ast ast needs to print
     * @param isIndent indicate if need indent in output
     * @return std::string printed string
     */
    std::string printAST(IAST *ast, bool isIndent = false) {
        buffer.clear();
        if (isIndent) {
            indent = true;
            cnt = 0;
        } else {
            indent = false;
        }
        ast->accept(this);
        std::string out(std::istreambuf_iterator<char>{buffer}, std::istreambuf_iterator<char>{});
        return out;
    }

  protected:
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
    VISIT_FUNCTION(BinOpExprAST) {
        buffer << "(" << v.op;
        cnt++;
        buffer << ", ";
        printIndent();
        v.lhs->accept(this);
        buffer << ", ";
        printIndent();
        v.rhs->accept(this);
        cnt--;
        printIndent();
        buffer << ")";
    }
    VISIT_FUNCTION(UnaryOpExprAST) {
        buffer << "(" << v.op;
        cnt++;
        buffer << ", ";
        printIndent();
        v.rhs->accept(this);
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
            buffer << "(\"";
            if (key) {
                key->accept(this);
            } else {
                buffer << "[UNSPECIFIED]";
            }
            buffer << "\", ";
            value->accept(this);
            buffer << ")";
        }
        cnt--;
        printIndent();
        buffer << ")";
    }
    VISIT_FUNCTION(LoopAST) {
        buffer << "(loop" << (v.label == "" ? "" : "@" + v.label) << ", ";
        cnt++;
        printIndent();
        buffer << "[init]";
        v.init->accept(this);
        buffer << ", ";
        printIndent();
        buffer << "[condition]";
        v.condition->accept(this);
        buffer << ", ";
        printIndent();
        buffer << "[return]";
        v.body->accept(this);
        cnt--;
        printIndent();
        buffer << ")";
    }
    VISIT_FUNCTION(BlockExprAST) {
        buffer << "(block";
        cnt++;
        for (auto &value : v.exprs) {
            buffer << ", ";
            printIndent();
            value->accept(this);
        }
        cnt--;
        printIndent();
        buffer << ")";
    }
    VISIT_FUNCTION(ControlFlowAST) {
        std::string type =
            std::map<ControlFlowAST::ControlFlowType, std::string>{
                {ControlFlowAST::ControlFlowType::BREAK, "BREAK"},
                {ControlFlowAST::ControlFlowType::CONTINUE, "CONTINUE"},
                {ControlFlowAST::ControlFlowType::RETURN, "RETURN"},
            }
                .find(v.controlFlowType)
                ->second;
        buffer << std::format("[{}]@{}", type, v.label);
    }
    VISIT_FUNCTION(TypeDefAST) {
        buffer << "(typeDef, ";
        cnt++;
        printIndent();
        buffer << v.name;
        buffer << ", ";
        printIndent();
        // buffer << v.definedType->toString();
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
        buffer << v.valueType->toString();
        buffer << ", ";
        printIndent();
        v.definedValue->accept(this);
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
        buffer << v.funcType->toString();
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
    VISIT_FUNCTION(SymbolDefAST) { buffer << "[SYMBOL DEF]"; }

  private:
    std::stringstream buffer; /*< print buffer*/
    bool indent;
    size_t cnt;
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