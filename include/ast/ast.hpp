#pragma once

#include <memory>
#include <vector>
#include <string>
#include <set>

#include "visitor.hpp"

#define ACCEPT_FUNCTION void accept(Visitor& v) override {v.visit(*this);}

namespace rulejit {

std::set<std::string> buildInType{
    "i64",
    "u64",
    "f64",
    "char",
};

struct AST{
    virtual void accept(Visitor&) = 0;
    virtual ~AST() = default;
};

struct ExprAST : public AST {
    std::string type;
};

struct IdentifierExprAST : public ExprAST{
    ACCEPT_FUNCTION;
    std::string name;
};

struct LiteralExprAST : public ExprAST{
    ACCEPT_FUNCTION;
    std::string value;
};

struct FunctionCallExprAST : public ExprAST{
    ACCEPT_FUNCTION;
    std::string functionName;
    std::vector<std::unique_ptr<AST>> params;
};

struct DefAST : public AST {
    std::string name;
};

struct TypeDefAST : public DefAST {
    ACCEPT_FUNCTION;
    struct VarDef {
        std::string type;
        std::string name;
    };
    std::vector<VarDef> member;
};

struct VarDefAST : public DefAST {
    ACCEPT_FUNCTION;
    std::string type;
    std::unique_ptr<ExprAST> defineValue;
};

struct FunctionDefAST : public DefAST {
    ACCEPT_FUNCTION;
    std::string type;
    std::vector<std::unique_ptr<IdentifierExprAST>> params;
    std::unique_ptr<ExprAST> returnValue;
};

}