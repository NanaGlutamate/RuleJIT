#pragma once

#include <memory>
#include <vector>
#include <string>
#include <set>
#include <algorithm>

#include "type.hpp"
#include "astvisitor.hpp"

#define ACCEPT_FUNCTION void accept(ASTVisitor& v) override {v.visit(*this);}

namespace rulejit {

struct AST{
    virtual void accept(ASTVisitor&) = 0;
    virtual ~AST() = default;
};

struct ExprAST : public AST {
    std::unique_ptr<TypeInfo> type;
};

struct AssignableExprAST : public AST {};

// x
struct IdentifierExprAST : public AssignableExprAST{
    ACCEPT_FUNCTION;
    std::string name;
};

// vector.x (same as vector["x"]) | list[a+b] | make_vector(1, 2, 3).x
struct MemberAccessExprAST : public AssignableExprAST{
    ACCEPT_FUNCTION;
    std::unique_ptr<ExprAST> baseVar;
    std::unique_ptr<ExprAST> memberToken;
};

// "abc" | 12 TODO:| 1e3
struct LiteralExprAST : public ExprAST{
    ACCEPT_FUNCTION;
    std::string value;
};

// add(1+3)
struct FunctionCallExprAST : public ExprAST{
    ACCEPT_FUNCTION;
    std::string functionName;
    std::vector<std::unique_ptr<ExprAST>> params;
};

// {1, 2, 3} | Info{Base{name: "abc", value: 3}, time: 13} | Vector3{x: 1, y: 2, z: 3}
struct ComplexLiteralExprAST : public ExprAST{
    ACCEPT_FUNCTION;
    std::vector<std::unique_ptr<ExprAST>> members;
};

// if(a==0){1}else{2}
struct BranchExprAST : public ExprAST{
    ACCEPT_FUNCTION;
    std::unique_ptr<ExprAST> condition;
    std::unique_ptr<ExprAST> trueBranch;
    std::unique_ptr<ExprAST> falseBranch;
};

// while(x!=0){x+=1;x;}
struct LoopAST : public ExprAST{
    ACCEPT_FUNCTION;
    std::unique_ptr<ExprAST> condition;
    std::unique_ptr<ExprAST> body;
};

// var x i64 = 12; x;
struct SequensialExprAST : public ExprAST{
    ACCEPT_FUNCTION;
    std::vector<std::unique_ptr<AST>> preStatement;
    std::unique_ptr<ExprAST> value;
};

// x = 12 CAUTION: no returns and not an expression
struct AssignmentAST : public AST{
    ACCEPT_FUNCTION;
    std::unique_ptr<AssignableExprAST> target;
    std::unique_ptr<ExprAST> value;
};

struct DefAST : public AST {
    std::string name;
};

// type Vector3 struct {x f64; y f64; z f64;} | type double f64 | TODO: type Reference<T> class {T item;}
struct TypeDefAST : public DefAST {
    ACCEPT_FUNCTION;
    struct VarDef {
        std::string type;
        std::string name;
    };
    std::vector<VarDef> member;
};

// var x []i64 = {1, 3, 4};
// TODO: var x [4]f64;
// TODO: var x []i64 {1, 3, 4};
struct VarDefAST : public DefAST {
    ACCEPT_FUNCTION;
    std::string type;
    std::unique_ptr<ExprAST> defineValue;
};

// func add(i i64) i64 -> i+1;
// TODO: | func add i64 -> i64 -> i64 = a -> b -> a+b
struct FunctionDefAST : public DefAST {
    ACCEPT_FUNCTION;
    std::string type;
    std::vector<std::unique_ptr<IdentifierExprAST>> params;
    // std::vector<std::unique_ptr<AST>> funcBody;
    std::unique_ptr<ExprAST> returnValue;
};

struct TopLevelAST : public AST{
    ACCEPT_FUNCTION;
    std::vector<std::unique_ptr<AST>> statements;
};

}