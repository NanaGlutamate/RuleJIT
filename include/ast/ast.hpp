#pragma once

#include <memory>
#include <vector>
#include <string>

#include "visitor.hpp"

#define ACCEPT_FUNCTION void accept(Visitor& v) override {v.visit(*this);}

namespace rulejit {

enum class VarType{
    REAL = 1,
    SIGNED = 2,
    UNSIGHED = 4,
    STRING = 8,
    FUNCTION = 16,
};

struct FunctionType{
    // f(A, B)->C = A->B->C
    std::vector<VarType> functionType;
};

struct AST{
    virtual void accept(Visitor&) = 0;
    virtual ~AST() = default;
};

struct ExprAST : public AST {
    VarType type;
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

struct VarDefAST : public DefAST {
    ACCEPT_FUNCTION;
    std::unique_ptr<ExprAST> defineValue;
};

struct FunctionDefAST : public DefAST {
    ACCEPT_FUNCTION;
    std::vector<std::unique_ptr<IdentifierExprAST>> params;
    std::unique_ptr<ExprAST> returnValue;
};

}